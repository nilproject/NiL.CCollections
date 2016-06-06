using System;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TestFileGenerator
{
    public sealed class WordSet : ISet<string>
    {
        private sealed class State
        {
            private sealed class _Comparer : Comparer<State>
            {
                public override int Compare(State x, State y)
                {
                    if (x == y)
                        return 0;
                    if (x == null)
                        return 1;
                    if (y == null)
                        return -1;
                    if (y.Char == x.Char)
                        return x.Childs.Count - y.Childs.Count;
                    return x.Char - y.Char;
                }
            }

            public static readonly Comparer<State> Comparer = new _Comparer();

            public readonly char Char;
            public byte ParentsSorted;
            internal byte deleted;
            public List<State> Parents;
            public readonly List<State> Childs;
            public int AcceptorPosition;

            public State()
            {
                Parents = new List<State>();
                Childs = new List<State>();
                ParentsSorted = 1;
            }

            public State(char c)
                : this()
            {
                Char = c;
            }

            public State(char c, State parent)
                : this(c)
            {
                Parents.Add(parent);
                parent.Childs.Add(this);
            }

            public override string ToString()
            {
                return (Char == 0 ? Parents.Count == 0 ? "<root>" : "<final>" : Char.ToString()) +
                    ":[" +
                    System.Linq.Enumerable.Aggregate(Childs,
                        "",
                        (r, x) => (string.IsNullOrEmpty(r) ? "" : r + ", ") +
                                  (x.Char == 0 ? "<final>" : x.Char.ToString())) +
                    "]";
            }
        }

        private State _root;
        private State _final;
        private int _count;
        private int _statesCount;
        private bool frozen;

        public int Count
        {
            get
            {
                return _count;
            }
        }

        public bool IsReadOnly
        {
            get
            {
                return false;
            }
        }

        public WordSet()
        {
            _root = new State();
            _final = new State();
        }

        public void Freeze()
        {
            if (frozen)
                return;

            Action<State> freeze = null;

            freeze = node =>
            {
                foreach (var child in node.Childs)
                    freeze(child);
                node.Parents = null;
            };

            frozen = true;
        }

        public void Compress()
        {
            var c = 0;
            while (merge(_final)) c++;
        }

        public Acceptor GetAcceptor()
        {
            var allocatedItems = 0;
            var items = new Acceptor.Item[(_statesCount >> 1) + _count];
            Func<State, int> addToAcceptor = null;
            //Dictionary<State, int> stateBlockPosition = new Dictionary<State, int>(items.Length);
            var maxIndex = 0;
            Action<State> reset = null;

            reset = node =>
            {
                for (var i = 0; i < node.Childs.Count; i++)
                    reset(node.Childs[i]);
                node.AcceptorPosition = 0;
            };

            reset(_root);

            addToAcceptor = state =>
            {
                int blockPosition = allocatedItems;
                var isFinal = false;
                //stateBlockPosition.Add(state, blockPosition);
                state.AcceptorPosition = blockPosition;
                allocatedItems += state.Childs.Count + 1;

                if (items.Length <= allocatedItems >> 1)
                {
                    var newItems = new Acceptor.Item[allocatedItems];
                    Array.Copy(items, newItems, items.Length);
                    items = newItems;
                }

                var index = 0;
                var i = 0;
                for (; i < state.Childs.Count; i++)
                {
                    index = blockPosition + i - (isFinal ? 1 : 0);
                    if (state.Childs[i] == _final)
                    {
                        isFinal = true;
                        continue;
                    }

                    items[index >> 1].Chars[index & 1] = state.Childs[i].Char;

                    int childBlockPosition = state.Childs[i].AcceptorPosition;
                    //if (!stateBlockPosition.TryGetValue(state.Childs[i], out childBlockPosition))
                    if (childBlockPosition == 0)
                        childBlockPosition = addToAcceptor(state.Childs[i]);

                    items[index >> 1].ChildsIndex[index & 1] = childBlockPosition;
                }

                if (isFinal)
                {
                    i--;
                    index = blockPosition + i;
                    items[index >> 1].ChildsIndex[index & 1] = -1;
                }

                maxIndex = Math.Max(index >> 1, maxIndex);

                return blockPosition;
            };

            addToAcceptor(_root);

            if (maxIndex + 2 != items.Length)
            {
                var newItems = new Acceptor.Item[maxIndex + 2];
                Array.Copy(items, newItems, newItems.Length);
                items = newItems;
            }

            return new Acceptor(items, _count);
        }

        public bool Add(string item)
        {
            if (item == null)
                throw new ArgumentNullException();

            if (frozen)
                throw new InvalidOperationException();

            bool needSplit = false;
            var node = _root;
            for (var i = 0; i < item.Length; i++)
            {
                bool found = false;

                for (var j = 0; j < node.Childs.Count; j++)
                {
                    if (node.Childs[j].Char == item[i])
                    {
                        found = true;
                        node = node.Childs[j];
                        needSplit |= node.Parents.Count > 1;
                        break;
                    }
                }

                if (!found)
                {
                    if (needSplit)
                    {
                        split(item);
                        return Add(item); // Рекурсия не будет более одного кадра глубиной
                    }

                    if (i == item.Length - 1)
                    {
                        var list = _final.Parents;
                        var len = list.Count;
                        for (var j = 0; j < len; j++)
                        {
                            if (list[j].Char == item[i])
                            {
                                if (list[j].Parents.IndexOf(node) == -1)
                                {
                                    list[j].Parents.Add(node);
                                    node.Childs.Add(list[j]);
                                }
                                _count++;
                                return true;
                            }
                        }
                    }

                    node = new State(item[i], node);

                    _statesCount++;
                    nodes++;
                }
            }

            if (node.Childs.Contains(_final))
                return false;

            if (needSplit)
            {
                split(item);
                return Add(item); // Рекурсия не будет более одного кадра глубиной
            }

            _count++;
            node.Childs.Add(_final);
            _final.Parents.Add(node);
            _final.ParentsSorted = 0;

            return true;
        }

        public int nodes = 0;
        public int merges = 0;
        public int splits = 0;

        private void split(string item)
        {
            var node = _root;
            for (var i = 0; i < item.Length; i++)
            {
                State child = null;
                for (var j = 0; j < node.Childs.Count; j++)
                {
                    if (node.Childs[j].Char == item[i])
                    {
                        child = node.Childs[j];
                        break;
                    }
                }

                if (child == null)
                    return;

                if (child.Parents.Count > 1)
                {
                    splits++;
                    _statesCount++;

                    child.Parents.RemoveAt(child.Parents.FindIndex(x => x == node));
                    node.Childs.RemoveAt(node.Childs.FindIndex(x => x == child));

                    var newState = new State(child.Char);
                    node.Childs.Add(newState);
                    newState.Parents.Add(node);

                    newState.Childs.AddRange(child.Childs);
                    for (var ci = 0; ci < newState.Childs.Count; ci++)
                    {
                        newState.Childs[ci].Parents.Add(newState);
                        newState.Childs[ci].ParentsSorted = 0;
                    }

                    child = newState;
                }

                node = child;
            }
        }

        private bool merge(State node)
        {
            bool result = false;
            int mergesCount = 0;
            var nodes = node.Parents;

            if (node.ParentsSorted == 0)
            {
                nodes.Sort(State.Comparer);
                node.ParentsSorted = 1;
            }

            var count = nodes.Count;
            for (var i = 1; i < count; i++)
            {
                if (nodes[i] == null || nodes[i].deleted != 0)
                {
                    mergesCount++;
                    nodes[i] = null;
                    continue;
                }

                if (node == _final)
                {
                    while (merge(nodes[i])) ;
                }

                for (var j = i; j-- > 0;)
                {
                    if (nodes[j] == null)
                        continue;

                    if (nodes[j].Char != nodes[i].Char)
                        break;

                    if (nodes[j].Childs.Count != nodes[i].Childs.Count)
                        break;

                    bool isEquals = true;
                    var len = nodes[j].Childs.Count;
                    var ichilds = nodes[i].Childs;
                    var jchilds = nodes[j].Childs;
                    for (var ci = 0; ci < len; ci++)
                    {
                        if (ichilds[ci] != jchilds[ci])
                        {
                            isEquals = false;
                            break;
                        }
                    }

                    if (isEquals)
                    {
                        merges++;
                        _statesCount--;
                        mergesCount++;
                        result = true;

                        for (var pi = 0; pi < nodes[i].Parents.Count; pi++)
                        {
                            nodes[i].Parents[pi].Childs.Remove(nodes[i]);
#if DEBUG
                            if (nodes[i].Parents[pi].Childs.FindIndex(x => x == nodes[j]) == -1)
                            {
                                if (nodes[j].Parents.FindIndex(x => x == nodes[i].Parents[pi]) != -1)
                                    throw new Exception();
                            }
                            else
                                throw new Exception();
#endif
                        }

                        for (var pi = 0; pi < nodes[i].Parents.Count; pi++)
                        {
#if DEBUG
                            if (nodes[i].Parents[pi].Childs.FindIndex(x => x == nodes[j]) == -1)
                            {
                                if (nodes[j].Parents.FindIndex(x => x == nodes[i].Parents[pi]) != -1)
                                    throw new Exception();
#endif
                                nodes[i].Parents[pi].Childs.Add(nodes[j]);
                                nodes[j].Parents.Add(nodes[i].Parents[pi]);
                                nodes[j].ParentsSorted = 0;
#if DEBUG
                            }
                            else
                                throw new Exception();
#endif
                        }

                        nodes[i].deleted = 1;
                        nodes[i] = null;

                        int c = 0;
                        while (merge(nodes[j])) c++;
                        break;
                    }
                }
            }

            if (mergesCount > 0)
            {
                nodes.RemoveAll(x => x == null);
            }

            return result;
        }

        public void UnionWith(IEnumerable<string> other)
        {
            throw new NotImplementedException();
        }

        public void IntersectWith(IEnumerable<string> other)
        {
            throw new NotImplementedException();
        }

        public void ExceptWith(IEnumerable<string> other)
        {
            throw new NotImplementedException();
        }

        public void SymmetricExceptWith(IEnumerable<string> other)
        {
            throw new NotImplementedException();
        }

        public bool IsSubsetOf(IEnumerable<string> other)
        {
            throw new NotImplementedException();
        }

        public bool IsSupersetOf(IEnumerable<string> other)
        {
            throw new NotImplementedException();
        }

        public bool IsProperSupersetOf(IEnumerable<string> other)
        {
            throw new NotImplementedException();
        }

        public bool IsProperSubsetOf(IEnumerable<string> other)
        {
            throw new NotImplementedException();
        }

        public bool Overlaps(IEnumerable<string> other)
        {
            throw new NotImplementedException();
        }

        public bool SetEquals(IEnumerable<string> other)
        {
            throw new NotImplementedException();
        }

        void ICollection<string>.Add(string item)
        {
            throw new NotImplementedException();
        }

        public void Clear()
        {
            _statesCount = 2;
            _count = 0;
            _root.Childs.Clear();
            _final.Parents?.Clear();
            _final.Childs?.Clear(); // о_О Такое возможно, так как в .NET строки могут содержать символ \0 не только в конце
        }

        public bool Contains(string item)
        {
            if (item == null)
                throw new ArgumentNullException();

            var node = _root;
            for (var i = 0; i < item.Length; i++)
            {
                bool found = false;
                for (var j = 0; j < node.Childs.Count; j++)
                {
                    if (node.Childs[j].Char == item[i])
                    {
                        found = true;
                        node = node.Childs[j];
                        break;
                    }
                }

                if (!found)
                    return false;
            }

            return node.Childs.Contains(_final);
        }

        public void CopyTo(string[] array, int arrayIndex)
        {
            throw new NotImplementedException();
        }

        public bool Remove(string item)
        {
            throw new NotImplementedException();
        }

        public IEnumerator<string> GetEnumerator()
        {
            throw new NotImplementedException();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            throw new NotImplementedException();
        }
    }
}

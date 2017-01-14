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
        private sealed class _Node
        {
            private sealed class _Comparer : Comparer<_Node>
            {
                public override int Compare(_Node x, _Node y)
                {
                    if (x == y)
                        return 0;

                    if (x == null)
                        return 1;

                    if (y == null)
                        return -1;

                    if (x.Char == y.Char)
                    {
                        if (x.Childs.Count != y.Childs.Count)
                            return x.Childs.Count - y.Childs.Count;

                        return x.Parents.Count - y.Parents.Count;
                    }

                    return x.Char - y.Char;
                }
            }

            public static readonly Comparer<_Node> Comparer = new _Comparer();

            public readonly char Char;
            public bool ParentsSorted;
            public bool Deleted;
            public List<_Node> Parents;
            public readonly List<_Node> Childs;

            public _Node()
            {
                Parents = new List<_Node>();
                Childs = new List<_Node>();
                ParentsSorted = true;
            }

            public _Node(char c)
                : this()
            {
                Char = c;
            }

            public _Node(char c, _Node parent)
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

        public int nodes = 0;
        public int merges = 0;
        public int splits = 0;

        private _Node _root;
        private _Node _final;
        private int _count;
        private int _statesCount;
        private bool frozen;

        public int FinalStateParentsCount => _final.Parents.Count;

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
            _root = new _Node();
            _final = new _Node();
        }

        public void Freeze()
        {
            if (frozen)
                return;

            Action<_Node> freeze = null;

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
            while (mergeParents(_final)) c++;
        }

        public Acceptor GetAcceptor()
        {
            var items = new Acceptor.Item[(_statesCount >> 1) + _count];
            var allocatedItems = 0;
            var maxIndex = 0;
            var acceptorPositions = new Dictionary<_Node, int>();

            Func<_Node, int> addToAcceptor = null;
            addToAcceptor = node =>
            {
                int blockPosition = allocatedItems;
                var isFinal = false;
                acceptorPositions[node] = blockPosition;
                allocatedItems += node.Childs.Count + 1;

                if (items.Length <= allocatedItems >> 1)
                {
                    var newItems = new Acceptor.Item[allocatedItems];
                    Array.Copy(items, newItems, items.Length);
                    items = newItems;
                }

                var index = 0;
                var i = 0;
                for (; i < node.Childs.Count; i++)
                {
                    index = blockPosition + i - (isFinal ? 1 : 0);
                    if (node.Childs[i] == _final)
                    {
                        isFinal = true;
                        continue;
                    }

                    items[index >> 1].Chars[index & 1] = node.Childs[i].Char;

                    int childBlockPosition = 0;
                    acceptorPositions.TryGetValue(node.Childs[i], out childBlockPosition);
                    if (childBlockPosition == 0)
                        childBlockPosition = addToAcceptor(node.Childs[i]);

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

            splitIfNeed(item);

            var node = _root;
            for (var i = 0; i < item.Length; i++)
            {
                var child = findChild(node, item[i]);

                if (child == null)
                {
                    if (i == item.Length - 1 && _final.ParentsSorted)
                    {
                        var parents = _final.Parents;
                        var len = parents.Count;
                        var j = findParentIndexForKey(parents, item[i]);
                        if (j >= 0)
                        {
                            for (; j < len; j++)
                            {
                                if (parents[j].Char != item[i])
                                    break;

                                if (parents[j].Childs.Count == 1 && parents[j].Childs[0] == _final)
                                {
                                    if (parents[j].Parents.IndexOf(node) == -1)
                                    {
                                        parents[j].Parents.Add(node);
                                        parents[j].ParentsSorted = false;
                                        node.Childs.Add(parents[j]);
                                    }

                                    _count++;
                                    return true;
                                }
                                else
                                {
                                    //break;
                                }
                            }
                        }
                    }

                    node = new _Node(item[i], node);

                    _statesCount++;
                    nodes++;
                }
                else
                {
                    node = child;
                }
            }

            if (node.Childs.Contains(_final))
                return false;

            _count++;
            node.Childs.Add(_final);
            _final.Parents.Add(node);
            _final.ParentsSorted = false;

            return true;
        }

        private int findParentIndexForKey(List<_Node> parents, char c)
        {
            if (parents.Count == 0 || parents[0].Char < c || parents[parents.Count - 1].Char > c)
                return -1;

            if (parents[0].Char == c)
                return 0;

            int result;
            if (parents[parents.Count - 1].Char == c)
            {
                result = parents.Count - 1;
            }
            else
            {
                var start = 0;
                var end = parents.Count - 1;
                result = start + (end - start) >> 1;

                while (end - start > 1)
                {
                    if (parents[result].Char < c)
                    {
                        end = result;
                    }
                    else if (parents[result].Char > c)
                    {
                        start = result;
                    }
                    else
                    {
                        break;
                    }

                    result = start + ((end - start) >> 1);
                }
            }

            while (result > 0 && parents[result - 1].Char == c)
                result--;

            return result;
        }

        private void splitIfNeed(string key)
        {
            var node = _root;
            for (var i = 0; i < key.Length; i++)
            {
                _Node child = findChild(node, key[i]);

                if (child == null)
                    return;

                if (child.Parents.Count > 1)
                {
                    splits++;
                    _statesCount++;

                    child.Parents.RemoveAt(child.Parents.IndexOf(node));
                    node.Childs.RemoveAt(node.Childs.IndexOf(child));

                    var newState = new _Node(child.Char);
                    node.Childs.Add(newState);
                    newState.Parents.Add(node);

                    newState.Childs.AddRange(child.Childs);
                    for (var ci = 0; ci < newState.Childs.Count; ci++)
                    {
                        newState.Childs[ci].Parents.Add(newState);
                        newState.Childs[ci].ParentsSorted = false;
                    }

                    child = newState;
                }

                node = child;
            }
        }

        private static _Node findChild(_Node node, char c)
        {
            for (var j = 0; j < node.Childs.Count; j++)
            {
                if (node.Childs[j].Char == c)
                {
                    return node.Childs[j];
                }
            }

            return null;
        }

        private bool mergeParents(_Node node)
        {
            bool result = false;
            int mergesCount = 0;
            var nodes = node.Parents;

            if (!node.ParentsSorted)
            {
                nodes.Sort(_Node.Comparer);
                nodes.Reverse();
                node.ParentsSorted = true;
            }
            else
            {
                return false;
            }

            var count = nodes.Count;
            for (var i = 1; i < count; i++)
            {
                if (nodes[i] == null || nodes[i].Deleted)
                {
                    mergesCount++;
                    nodes[i] = null;
                    continue;
                }

                if (node == _final)
                {
                    while (mergeParents(nodes[i])) ;
                }

                for (var j = i; j-- > 0;)
                {
                    if (nodes[j] == null)
                        continue;

                    bool isEquals = true;

                    if (nodes[i] != nodes[j])
                    {
                        if (nodes[j].Char != nodes[i].Char)
                            break;

                        if (nodes[j].Childs.Count != nodes[i].Childs.Count)
                            break;

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
                            nodes[j].Parents.Capacity = Math.Max(nodes[j].Parents.Capacity, nodes[j].Parents.Count + nodes[i].Parents.Count);
                            nodes[j].Parents.Add(nodes[i].Parents[pi]);
                            nodes[j].ParentsSorted = false;
#if DEBUG
                            }
                            else
                                throw new Exception();
#endif
                        }

                        nodes[i].Deleted = true;
                        nodes[i] = null;

                        //while (merge(nodes[j])) ;
                        break;
                    }
                }
            }

            for (var i = 0; i < nodes.Count; i++)
            {
                if (nodes[i] != null && !nodes[i].Deleted)
                    while (mergeParents(nodes[i])) ;
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

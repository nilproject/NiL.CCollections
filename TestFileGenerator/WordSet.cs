using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace TestFileGenerator
{
    public sealed class WordSet : ISet<string>
    {
        private sealed class State
        {
            public char Char { get; private set; }
            public List<State> Parents;
            public readonly List<State> Childs;

            public State()
            {
                Parents = new List<State>();
                Childs = new List<State>();
            }

            public State(char c, State parent)
                : this()
            {
                Char = c;
                Parents.Add(parent);
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
        private bool _emptyKeyContains;
        private int _count;
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
            while (merge(_final)) ;
        }

        public bool Add(string item)
        {
            if (item == null)
                throw new ArgumentNullException();

            if (frozen)
                throw new InvalidOperationException();

            if (item.Length == 0)
            {
                if (!_emptyKeyContains)
                {
                    _emptyKeyContains = true;
                    _count++;
                    return true;
                }

                return false;
            }

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

                    node.Childs.Add(node = new State(item[i], node));
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

                    child.Parents.Remove(node);
                    node.Childs.Remove(child);

                    var newState = new State(child.Char, node);
                    newState.Childs.AddRange(child.Childs);
                    node.Childs.Add(newState);
                    for (var ci = 0; ci < newState.Childs.Count; ci++)
                    {
                        newState.Childs[ci].Parents.Add(newState);
                    }
                }

                node = child;
            }
        }

        private bool merge(State node)
        {
            bool tail = false;
            bool result = false;
            do
            {
                tail = false;
                bool merged = false;
                var nodes = node.Parents;
                nodes.Sort((x, y) => x == y ? 0 : x == null ? 1 : y == null ? -1 : x.Char == y.Char ? x.Childs.Count - y.Childs.Count : x.Char - y.Char);
                for (var i = 1; i < nodes.Count; i++)
                {
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
                            merged = true;
                            result = true;

                            var iparents = nodes[i].Parents;
                            for (var pi = 0; pi < iparents.Count; pi++)
                            {
                                if (!iparents[pi].Childs.Contains(nodes[j]))
                                {
                                    iparents[pi].Childs.Add(nodes[j]);
                                    nodes[j].Parents.Add(iparents[pi]);
                                }

                                iparents[pi].Childs.Remove(nodes[i]);
                            }

                            nodes[i] = null;
                            //node = nodes[j];
                            //tail = true;

                            while (merge(nodes[j])) ;
                            break;
                        }
                    }
                }

                if (merged)
                {
                    nodes.RemoveAll(x => x == null);
                    nodes.TrimExcess();
                }
            }
            while (tail);

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
            throw new NotImplementedException();
        }

        public bool Contains(string item)
        {
            if (item == null)
                throw new ArgumentNullException();

            if (item.Length == 0)
            {
                return _emptyKeyContains;
            }

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

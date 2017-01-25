using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TestFileGenerator
{
    public sealed class Acceptor : ISet<string>
    {
        private Item[] _items;
        private int _count;

        public struct TwoItemsArray<T> where T : struct
        {
            public T item0, item1;
            public T this[int index]
            {
                get
                {
                    if (index == 0)
                        return item0;
                    if (index == 1)
                        return item1;
                    throw new ArgumentOutOfRangeException();
                }
                set
                {
                    if (index == 0)
                        item0 = value;
                    else if (index == 1)
                        item1 = value;
                    else
                        throw new ArgumentOutOfRangeException();
                }
            }
        }

        public struct Item
        {
            public TwoItemsArray<char> Chars;
            public TwoItemsArray<int> ChildsIndex;

            public override string ToString()
            {
                return Chars[0] + ", " + Chars[1];
            }
        }

        internal Acceptor(Item[] items, int count)
        {
            if (items == null)
                throw new ArgumentNullException();

            _items = items;
            _count = count;
        }

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
                return true;
            }
        }

        public bool Add(string item)
        {
            throw new InvalidOperationException();
        }

        public void Clear()
        {
            throw new InvalidOperationException();
        }

        public bool Contains(string item)
        {
            if (item == null)
                throw new ArgumentNullException();

            var keyCharIndex = 0;
            var nodeIndex = 0;
            while (keyCharIndex < item.Length)
            {
                while (_items[nodeIndex >> 1].Chars[nodeIndex & 1] != 0 && _items[nodeIndex >> 1].Chars[nodeIndex & 1] != item[keyCharIndex])
                    nodeIndex++;

                if (_items[nodeIndex >> 1].Chars[nodeIndex & 1] == 0)
                    return false;

                nodeIndex = _items[nodeIndex >> 1].ChildsIndex[nodeIndex & 1];
                keyCharIndex++;
            }

            while (_items[nodeIndex >> 1].Chars[nodeIndex & 1] != 0)
                nodeIndex++;

            return _items[nodeIndex >> 1].ChildsIndex[nodeIndex & 1] != 0;
        }

        public void CopyTo(string[] array, int arrayIndex)
        {
            throw new NotImplementedException();
        }

        public void ExceptWith(IEnumerable<string> other)
        {
            throw new NotImplementedException();
        }

        public IEnumerator<string> GetEnumerator()
        {
            if (_count == 0)
                yield break;

            var stack = new Stack<int>();
            var result = new StringBuilder();

            stack.Push(0);
            do
            {
                var nodeIndex = stack.Pop();

                if (_items[nodeIndex >> 1].ChildsIndex[nodeIndex & 1] == -1)
                {
                    yield return result.ToString();
                    nodeIndex++;
                }

                if (_items[nodeIndex >> 1].Chars[nodeIndex & 1] != 0)
                {
                    result.Append(_items[nodeIndex >> 1].Chars[nodeIndex & 1]);
                    stack.Push(nodeIndex + 1);
                    stack.Push(_items[nodeIndex >> 1].ChildsIndex[nodeIndex & 1]);
                    continue;
                }

                if (result.Length > 0)
                    result.Length--;
            }
            while (stack.Count != 0);
        }

        public void IntersectWith(IEnumerable<string> other)
        {
            throw new NotImplementedException();
        }

        public bool IsProperSubsetOf(IEnumerable<string> other)
        {
            throw new NotImplementedException();
        }

        public bool IsProperSupersetOf(IEnumerable<string> other)
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

        public bool Overlaps(IEnumerable<string> other)
        {
            throw new NotImplementedException();
        }

        public bool Remove(string item)
        {
            throw new NotImplementedException();
        }

        public bool SetEquals(IEnumerable<string> other)
        {
            throw new NotImplementedException();
        }

        public void SymmetricExceptWith(IEnumerable<string> other)
        {
            throw new NotImplementedException();
        }

        public void UnionWith(IEnumerable<string> other)
        {
            throw new NotImplementedException();
        }

        void ICollection<string>.Add(string item)
        {
            throw new NotImplementedException();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            throw new NotImplementedException();
        }
    }
}

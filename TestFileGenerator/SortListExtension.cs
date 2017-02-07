using System;
using System.Collections.Generic;

namespace TestFileGenerator
{
    public static class SortListExtension
    {
        public static void InPlaceMergeSort<T>(this IList<T> list) where T : IComparable<T>
        {
            for (var partLen = 2; partLen <= list.Count; partLen *= 2)
            {
                for (var part = 0; part < list.Count / partLen; part++)
                {
                    var prewPartLen = partLen / 2;
                    if (partLen == 2)
                    {
                        if (list[part * partLen].CompareTo(list[prewPartLen + part * partLen]) > 0)
                        {
                            var temp = list[part * partLen];
                            list[part * partLen] = list[prewPartLen + part * partLen];
                            list[prewPartLen + part * partLen] = temp;
                        }
                    }
                    else
                    {
                        var relocatedIndex = 0;
                        var relocatedCount = 0;
                        T temp;
                        do
                        {
                            var i = relocatedIndex;
                            var j = prewPartLen;
                            relocatedIndex = 0;
                            relocatedCount = 0;
                            for (; i < prewPartLen && j < partLen; i++)
                            {
                                if (relocatedIndex > 0)
                                {
                                    var cmp = list[relocatedIndex + part * partLen].CompareTo(list[j + part * partLen]);
                                    if (cmp <= 0)
                                    {
                                        temp = list[i + part * partLen];
                                        list[i + part * partLen] = list[relocatedIndex + part * partLen];
                                        list[relocatedIndex + part * partLen] = temp;

                                        for (var k = 0; k < relocatedCount - 1; k++)
                                        {
                                            temp = list[k + relocatedIndex + part * partLen];
                                            list[k + relocatedIndex + part * partLen] = list[k + 1 + relocatedIndex + part * partLen];
                                            list[k + 1 + relocatedIndex + part * partLen] = temp;
                                        }
                                    }
                                    else if (cmp > 0)
                                    {
                                        temp = list[i + part * partLen];
                                        list[i + part * partLen] = list[j + part * partLen];
                                        list[j + part * partLen] = temp;

                                        relocatedCount++;
                                        j++;
                                    }
                                }
                                else
                                {
                                    var cmp = list[i + part * partLen].CompareTo(list[j + part * partLen]);
                                    if (cmp > 0)
                                    {
                                        temp = list[i + part * partLen];
                                        list[i + part * partLen] = list[j + part * partLen];
                                        list[j + part * partLen] = temp;

                                        relocatedIndex = j;
                                        relocatedCount++;
                                        j++;
                                    }
                                }
                            }

                            if (j == partLen)
                            {
                                relocatedIndex = i;
                                continue;
                            }

                            prewPartLen += relocatedCount;
                        }
                        while (relocatedCount > 0);
                    }
                }
            }
        }

        public static void ShellSort<T>(this IList<T> list) where T : IComparable<T>
        {
            var d = list.Count / 2;
            while (d > 1)
            {
                for (var i = d; i < list.Count; i++)
                {
                    var cmp = list[i - d].CompareTo(list[i]);
                    if (cmp > 0)
                    {
                        var temp = list[i - d];
                        list[i - d] = list[i];
                        list[i] = temp;
                    }
                }

                d = d * 15 / 18;
            }

            InsertionSort(list);
        }

        private static int computeSedjvikSequenceIndex<T>(IList<T> list) where T : IComparable<T>
        {
            var di = 0;
            var minDi = 0;
            var maxDi = 10;
            while (sedjvikSequence(maxDi) * 3 < list.Count)
            {
                minDi = maxDi;
                maxDi += 5;
            }

            int t;
            do
            {
                t = sedjvikSequence(minDi + (maxDi - minDi) / 2) * 3;
                if (t == list.Count)
                {
                    minDi = maxDi = minDi + (maxDi - minDi) / 2;
                    di = t;
                }
                else if (t < list.Count)
                {
                    minDi = minDi + (maxDi - minDi) / 2;
                }
                else
                {
                    maxDi = minDi + (maxDi - minDi) / 2;
                }
            }
            while (maxDi - minDi > 1);

            if (t > list.Count)
            {
                di = minDi;
            }
            else
            {
                di = maxDi;
            }

            return di;
        }

        public static void InsertionSort<T>(this IList<T> list) where T : IComparable<T>
        {
            for (var i = 1; i < list.Count; i++)
            {
                var cmp = list[i].CompareTo(list[i - 1]);
                if (cmp < 0)
                {
                    var newIndex = binarySearchMore(list, list[i], i);

                    if (newIndex == -1)
                        newIndex = i;

                    var t = list[i];

                    for (var j = i; j > newIndex; j--)
                    {
                        list[j] = list[j - 1];
                    }

                    list[newIndex] = t;
                }
            }
        }

        private static int binarySearchMore<T>(IList<T> list, T value, int length) where T : IComparable<T>
        {
            if (length <= 0)
                return -1;

            if (length <= 3)
            {
                for (var i = 0; i < length; i++)
                {
                    if (list[i].CompareTo(value) > 0)
                        return i;
                }

                return -1;
            }

            var start = 0;
            var end = length - 1;
            var index = start + ((end - start) >> 1);

            if (list[end].CompareTo(value) <= 0)
                return -1;

            if (list[0].CompareTo(value) > 0)
                return 0;

            for (;;)
            {
                var item = list[index];
                var cmp = item.CompareTo(value);

                if (end - start == 1)
                {
                    if (cmp <= 0)
                        index++;
                    break;
                }

                if (cmp > 0)
                {
                    end = index;
                }
                else if (cmp <= 0)
                {
                    start = index;
                }

                index = start + ((end - start) >> 1);
            }

            return index;
        }

        private static int sedjvikSequence(int i)
        {
            return (i & 1) == 0 ? 9 * (1 << i) - 9 * (1 << (i / 2)) + 1 : 8 * (1 << i) - 6 * (1 << ((i + 1) / 2)) + 1;
        }
    }
}

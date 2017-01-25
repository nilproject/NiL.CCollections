using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace TestFileGenerator
{
    class Program
    {
        static void Main(string[] args)
        {
            simpleList();
        }

        private static void simpleList()
        {
            var words = new HashSet<string>();
            var stringBuilder = new StringBuilder();
            var size = 0;
            var count = 0;
            var uncompressedSize = 0;
            var limit = 32 * 1024 * 1024;
            var random = new Random(777);
            var updated = Environment.TickCount;
            var sw = Stopwatch.StartNew();

            while (size <= limit)
            {
                var len = random.Next(10) + 3;

                for (var i = 0; i < len; i++)
                {
                    stringBuilder.Append((char)(random.Next(128 - 32) + 32));
                }

                var word = stringBuilder.ToString();
                stringBuilder.Clear();

                if (words.Add(word))
                {
                    //writer.Write(word);
                    //writer.Write('\n');
                    size += len + 1;
                    uncompressedSize += len + 1;
                    count++;
                }

                if (Environment.TickCount - updated > 500)
                {
                    updated = Environment.TickCount;
                    Console.SetCursorPosition(0, 0);
                    Console.Write(((float)size / limit).ToString("0.00000"));
                }
            }

            sw.Stop();
            Console.WriteLine();
            Console.WriteLine(sw.Elapsed);

            sw.Restart();
            var wordList = words.ToArray();
            words.Clear();
            words = null;
            Array.Sort(wordList);
            sw.Stop();

            Console.WriteLine(sw.Elapsed);

            GC.Collect(2);
            GC.WaitForFullGCComplete();

            Console.WriteLine(wordList.Length);

            sw.Restart();
            for (var i = 0; i < wordList.Length; i++)
            {
                Array.BinarySearch(wordList, wordList[i]);
            }
            sw.Stop();
            Console.WriteLine(sw.Elapsed);
            Console.WriteLine((sw.Elapsed.TotalMilliseconds / (float)count).ToString("F10"));

            Console.ReadLine();
        }

        private static void validation()
        {
            var words = new WordSet();
            //words.Add("fox");
            words.Add("foxes");
            words.Add("boxes");

            words.Compress();
            var acceptor = words.GetAcceptor();

            foreach (var str in acceptor)
                Console.WriteLine(str);
        }

        private static void performance()
        {
            //using (var file = new FileStream("words.txt", FileMode.Create))
            //using (var writer = new StreamWriter(file))

            var words = new WordSet();
            //var words = new HashSet<string>();
            var stringBuilder = new StringBuilder();
            var size = 0;
            var count = 0;
            var uncompressedSize = 0;
            var limit = 32 * 1024 * 1024;
            var random = new Random(777);
            var updated = Environment.TickCount;
            var sw = Stopwatch.StartNew();
            var acceptorSize = 0;
            var acceptors = new List<Acceptor>();
            var strings = new HashSet<string>();

            while (size <= limit)
            {
                var len = random.Next(10) + 3;

                for (var i = 0; i < len; i++)
                {
                    stringBuilder.Append((char)(random.Next(128 - 32) + 32));
                }

                var word = stringBuilder.ToString();
                stringBuilder.Clear();

                if (words.Add(word))
                {
                    //writer.Write(word);
                    //writer.Write('\n');
                    size += len + 1;
                    uncompressedSize += len + 1;
                    acceptorSize += len + 1;
                    strings.Add(word);
                    count++;
                }

                //if (words.Count % 500000 == 0)
                if (uncompressedSize >= (4 * 128 * 1024))
                //if (uncompressedSize >= 10000)
                {
                    words.Compress();
                    uncompressedSize = 0;
                }

                if (acceptorSize >= 16 * 1024 * 1024)
                {
                    acceptorSize = 0;
                    words.Compress();
                    acceptors.Add(words.GetAcceptor());
                    words.Clear();
                }

                if (Environment.TickCount - updated > 500)
                {
                    updated = Environment.TickCount;
                    Console.SetCursorPosition(0, 0);
                    Console.Write(((float)size / limit).ToString("0.00000"));
                }
            }


            words.Compress();
            acceptors.Add(words.GetAcceptor());

            sw.Stop();

            words.Clear();
            words = null;
            stringBuilder = null;
            random = null;

            GC.Collect();
            GC.WaitForPendingFinalizers();
            GC.Collect();

            Console.WriteLine();
            Console.WriteLine(sw.Elapsed);
            Console.WriteLine((sw.Elapsed.TotalMilliseconds / (float)count).ToString("F10"));
            Console.WriteLine("Lines: " + count);

            sw.Restart();
            foreach (var word in strings)
            {
                bool found = false;

                for (var i = 0; i < acceptors.Count; i++)
                {
                    if (acceptors[i].Contains(word))
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                    throw new KeyNotFoundException(word);
            }
            sw.Stop();
            Console.WriteLine(sw.Elapsed);
            Console.WriteLine((sw.Elapsed.TotalMilliseconds / (float)count).ToString("F10"));

            foreach (var acceptor in acceptors)
            {
                foreach (var word in acceptor)
                {
                    if (!strings.Contains(word))
                        throw new KeyNotFoundException(word);
                }
            }
            strings.Clear();

            GC.Collect();
            GC.WaitForPendingFinalizers();
            GC.Collect();
            Console.ReadKey();
        }
    }
}

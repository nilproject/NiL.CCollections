using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TestFileGenerator
{
    class Program
    {
        static void Main(string[] args)
        {
            //using (var file = new FileStream("words.txt", FileMode.Create))
            //using (var writer = new StreamWriter(file))
            {
                var words = new WordSet();
                /*
                words.Add("foxes");
                words.Add("fox");
                words.Add("boxes");

                words.Merge();

                Console.WriteLine(words.Contains("fox"));
                Console.WriteLine(words.Contains("foxes"));
                Console.WriteLine(words.Contains("box"));
                Console.WriteLine(words.Contains("boxes"));

                return;
                */
                var stringBuilder = new StringBuilder();
                var size = 0;
                var limit = 3 * 1024 * 1024;
                var random = new Random(Environment.TickCount);
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
                    }

                    if (Environment.TickCount - updated > 500)
                    {
                        updated = Environment.TickCount;
                        Console.SetCursorPosition(0, 0);
                        Console.Write(((float)size / limit).ToString("0.00000"));

                        //words.Compress();

                        if (GC.GetTotalMemory(false) > 1024 * 1024 * 1024)
                        {
                            //GC.Collect();
                            //GC.WaitForPendingFinalizers();
                            //GC.Collect();
                            break;
                        }
                    }
                }
                
                words.Compress();
                words.Freeze();

                sw.Stop();

                GC.Collect();
                GC.WaitForPendingFinalizers();
                GC.Collect();

                Console.WriteLine();
                Console.WriteLine(sw.Elapsed);
                Console.WriteLine((sw.Elapsed.TotalMilliseconds / (float)words.Count).ToString("F10"));
                Console.WriteLine("Nodes: " + words.nodes);
                Console.WriteLine("Merges: " + words.merges);
                Console.WriteLine("Splits: " + words.splits);


                /*var sw = Stopwatch.StartNew();
                foreach (var word in words)
                {
                    if (!words.Contains(word))
                        throw new KeyNotFoundException(word);
                }

                Console.WriteLine(sw.Elapsed);
                Console.WriteLine((sw.Elapsed.TotalMilliseconds / (float)words.Count).ToString("F10"));
                */

                GC.Collect();
                GC.WaitForPendingFinalizers();
                GC.Collect();
                Console.ReadKey();
                Console.WriteLine(words.Count);
                Console.ReadKey();
            }
        }
    }
}

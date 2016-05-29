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
                var words = new HashSet<string>();
                var stringBuilder = new StringBuilder();
                var size = 0;
                var random = new Random(Environment.TickCount);

                while (size <= 128 * 1024 * 1024)
                {
                    var len = random.Next(10) + 3;

                    for (var i = 0; i < len; i++)
                    {
                        stringBuilder.Append((char)(random.Next(128 - 32) + 32));
                    }

                    var word = stringBuilder.ToString();
                    stringBuilder.Clear();

                    if (words.Contains(word))
                        continue;
                    else
                    {
                        words.Add(word);
                        //writer.Write(word);
                        //writer.Write('\n');
                        size += len + 1;
                    }
                }

                var sw = Stopwatch.StartNew();
                foreach (var word in words)
                {
                    words.Contains(word);
                }
                Console.WriteLine(sw.Elapsed);
                Console.WriteLine((sw.Elapsed.TotalMilliseconds / (float)words.Count).ToString("F10"));
            }
        }
    }
}

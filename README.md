<div style="border-left: 4px solid #007acc; padding: 0px; background-color: #f0f8ff;">
<div style="padding:16px; font-family: system-ui, -apple-system, 'Segoe UI', Roboto, Arial, sans-serif; color:#0b1220; line-height:1.45;">
<h1 style="margin:0 0 8px 0;">TickerTape</h1>
<p style="margin:0 0 12px 0;">Quick demonstration mixing modern C++ elements/containers (tuple, map, vector) with <code>struct</code>. Includes C++ examples and illustrative x64 assembly snippets. Code blocks below are indented and marked with <code>language-cpp</code> / <code>language-asm</code> classes for syntax highlighting by client-side highlighters; styling is provided for readable, copy‑ready presentation.</p>

<style>
/* Basic code block styling and lightweight keyword coloring for readability /
.code-block { background:#ffffff; padding:12px; border-radius:4px; overflow:auto; font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, "Roboto Mono", "Segoe UI Mono", monospace; font-size:13px; color:#0b1220; border:1px solid rgba(0,0,0,0.06); }
.language-cpp .kw { color:#00008b; font-weight:600; } / keywords /
.language-cpp .type { color:#2b6f44; font-weight:600; } / types /
.language-cpp .str { color:#a31515; } / strings */
.language-cpp .comment { color:#6a737d; font-style:italic; }
.language-asm .comment { color:#6a737d; font-style:italic; }
pre { margin:12px 0; white-space:pre; }
h2 { margin:12px 0 6px 0; }
h3 { margin:10px 0 6px 0; }
ul { margin:8px 0 12px 20px; }
</style>

<hr style="border:none;border-top:1px solid rgba(11,18,32,0.06);margin:12px 0;">

<h2>C++ Lambda Capture Block</h2>
<p>Capture variables inside square brackets <code>[]</code>. Example:</p>

<pre class="code-block language-cpp"><code>
<span class="kw">auto</span> fn = [x, &amp;y](<span class="type">int</span> z) {
<span class="kw">return</span> x + y + z;
};
</code></pre>

<h2>Example Output (value parameters)</h2>
<pre class="code-block"><code>in=1 out=-1 filename=C:\Users\Jerry\Downloads\SymbolsURLs.txt
in=2 out=-2 filename=C:\Users\Jerry\Downloads\StocksURLs.txt
in=3 out=-3 filename=C:\Users\Jerry\Downloads\CombinedStocks.csv
in=4 out=-4 filename=C:\Users\Jerry\Downloads\CombinedStocks.csv</code></pre>

<h2>cleanFiles — value parameters</h2>
<p>Deletes a list of files and reports progress. Lambda <code>tryDelete</code> updates local counters passed by value.</p>

<pre class="code-block language-cpp"><code>
<span class="kw">static</span> <span class="type">bool</span> cleanFiles(std::initializer_list&lt;string&gt; filenames, <span class="type">int</span> in = 0, <span class="type">int</span> out = 0)
{
<span class="kw">namespace</span> fs = std::filesystem;
<span class="kw">auto</span> tryDelete = [&amp;in, &amp;out](<span class="type">const string&amp;</span> filename) -&gt; <span class="type">bool</span>
{
in++;
--out;
cout &lt;&lt; <span class="str">"in="</span> &lt;&lt; in &lt;&lt; <span class="str">" out="</span> &lt;&lt; out &lt;&lt; <span class="str">" filename="</span> &lt;&lt; filename &lt;&lt; endl;
std::error_code ec;
<span class="kw">if</span> (fs::exists(filename, ec))
{
fs::remove(filename, ec);
<span class="kw">return</span> !ec;
}
<span class="kw">return</span> <span class="kw">true</span>;
};
<span class="type">bool</span> lOK = <span class="kw">true</span>;
<span class="kw">for</span> (const auto&amp; filename : filenames)
lOK &amp;= tryDelete(filename);
<span class="kw">return</span> lOK;
}
</code></pre>

<h3>Usage (value example)</h3>
<pre class="code-block language-cpp"><code>
<span class="type">bool</span> downloadDataset(
Stock&amp; stocks,
map&lt;string, string&gt;&amp; symbols,
_TICKER_TAPE_ARGS&amp; args
)
{
<span class="kw">if</span> (args.bCleanApp)
{
cout &lt;&lt; <span class="str">"Deleting old files..."</span>;
cleanFiles({
fullpathSymbolsURLsFilename,
fullpathStocksURLsFilename,
fullpathCombinedStocksFilename,
fullpathParseStocksFilename
});
cout &lt;&lt; <span class="str">"done."</span> &lt;&lt; endl;
}
<span class="kw">return</span> <span class="kw">true</span>;
}
</code></pre>

<hr style="border:none;border-top:1px solid rgba(11,18,32,0.06);margin:12px 0;">

<h2>x64 Assembly (illustrative) — value version</h2>
<p><strong>Notes:</strong> MSVC x64 calling convention: <code>RCX</code>=pointer, <code>RDX</code>=count. Shadow space reserved. Adjust <code>sizeof(std::string)</code> per your STL implementation.</p>

<pre class="code-block language-asm"><code>
; bool cleanFiles(std::initializer_list&lt;std::string&gt; filenames, int in = 0, int out = 0)
; RCX = filenames._First  (std::string*)
; RDX = filenames._Count  (size_t)
EXTERN fs_exists_with_ec : PROC
EXTERN fs_remove_with_ec : PROC
EXTERN print_status      : PROC
cleanFiles PROC
push rbp
mov  rbp, rsp
sub  rsp, 32
xor  eax, eax
mov  qword ptr [rbp-24], rax
mov  qword ptr [rbp-32], rax
mov  dword ptr [rbp-4], 1
xor  r8, r8
mov  qword ptr [rbp-16], r8
mov  rdi, rcx
mov  rsi, rdx
for_loop_start:
mov  rax, qword ptr [rbp-16]
cmp  rax, rsi
jae  for_loop_end
mov  rcx, rax
shl  rcx, 5
lea  rbx, [rdi + rcx]
mov  rdx, qword ptr [rbp-24]
inc  rdx
mov  qword ptr [rbp-24], rdx
mov  r8,  qword ptr [rbp-32]
dec  r8
mov  qword ptr [rbp-32], r8
mov  ecx, dword ptr [rbp-24]
mov  edx, dword ptr [rbp-32]
mov  r8,  rbx
call print_status
lea  rdx, [rsp]
mov  rcx, rbx
call fs_exists_with_ec
test al, al
jz   tryDelete_return_true
lea  rdx, [rsp]
mov  rcx, rbx
call fs_remove_with_ec
jmp  tryDelete_after
tryDelete_return_true:
mov  al, 1
tryDelete_after:
mov  ecx, dword ptr [rbp-4]
and  cl, al
mov  dword ptr [rbp-4], ecx
mov  rax, qword ptr [rbp-16]
inc  rax
mov  qword ptr [rbp-16], rax
jmp  for_loop_start
for_loop_end:
mov  eax, dword ptr [rbp-4]
and  eax, 1
mov  rsp, rbp
pop  rbp
ret
cleanFiles ENDP
</code></pre>

<hr style="border:none;border-top:1px solid rgba(11,18,32,0.06);margin:12px 0;">

<h2>Reference-parameter variant</h2>
<p>Pass counters by reference to preserve and observe caller-side changes.</p>

<h3>Example output (reference)</h3>
<pre class="code-block"><code>in=101 out=-101 filename=C:\Users\Jerry\Downloads\SymbolsURLs.txt
in=102 out=-102 filename=C:\Users\Jerry\Downloads\StocksURLs.txt
in=103 out=-103 filename=C:\Users\Jerry\Downloads\CombinedStocks.csv
in=104 out=-104 filename=C:\Users\Jerry\Downloads\CombinedStocks.csv</code></pre>

<h3>cleanFiles — reference parameters</h3>
<pre class="code-block language-cpp"><code>
<span class="kw">static</span> <span class="type">bool</span> cleanFiles(std::initializer_list&lt;string&gt; filenames, <span class="type">int&amp;</span> in, <span class="type">int&amp;</span> out)
{
<span class="kw">namespace</span> fs = std::filesystem;
<span class="kw">auto</span> tryDelete = [&amp;in, &amp;out](<span class="type">const string&amp;</span> filename) -&gt; <span class="type">bool</span>
{
in++;
--out;
cout &lt;&lt; <span class="str">"in="</span> &lt;&lt; in &lt;&lt; <span class="str">" out="</span> &lt;&lt; out &lt;&lt; <span class="str">" filename="</span> &lt;&lt; filename &lt;&lt; endl;
std::error_code ec;
<span class="kw">if</span> (fs::exists(filename, ec))
{
fs::remove(filename, ec);
<span class="kw">return</span> !ec;
}
<span class="kw">return</span> <span class="kw">true</span>;
};
<span class="type">bool</span> lOK = <span class="kw">true</span>;
<span class="kw">for</span> (const auto&amp; filename : filenames)
lOK &amp;= tryDelete(filename);
<span class="kw">return</span> lOK;
}
</code></pre>

<h3>Usage (reference example)</h3>
<pre class="code-block language-cpp"><code>
<span class="type">bool</span> downloadDataset(
Stock&amp; stocks,
map&lt;string, string&gt;&amp; symbols,
_TICKER_TAPE_ARGS&amp; args
)
{
<span class="type">int</span> in = 100;
<span class="type">int</span> out = -100;
<span class="kw">if</span> (args.bCleanApp)
{
cout &lt;&lt; <span class="str">"Deleting old files..."</span>;
cleanFiles({
fullpathSymbolsURLsFilename,
fullpathStocksURLsFilename,
fullpathCombinedStocksFilename,
fullpathParseStocksFilename
},
in,
out
);
cout &lt;&lt; <span class="str">"done"</span> &lt;&lt; endl;
}
<span class="kw">return</span> <span class="kw">true</span>;
}
</code></pre>

<h2>x64 Assembly Notes (reference)</h2>
<p><strong>Preserved behavior</strong></p>
<ul>
<li>Reference-based updates: <code>in</code> and <code>out</code> mutated via pointers (R8, R9).</li>
<li><code>initializer_list</code> ABI: pointer + count in RCX/RDX.</li>
<li>Lambda inlining: captured refs are pointers; no closure object required.</li>
<li>Filesystem calls abstracted to helper routines for <code>exists</code> and <code>remove</code>.</li>
<li>Proper stack frame and nonvolatile register preservation illustrated.</li>
</ul>

<pre class="code-block language-asm"><code>
; bool cleanFiles(std::initializer_list&lt;string&gt; filenames, int&amp; in, int&amp; out)
; RCX = filenames._First (std::string*)
; RDX = filenames._Count (size_t)
; R8  = &amp;in
; R9  = &amp;out
EXTERN fs_exists_with_ec : PROC
EXTERN fs_remove_with_ec : PROC
EXTERN print_status      : PROC
cleanFiles PROC
push rbp
mov  rbp, rsp
sub  rsp, 32
push rsi
push rdi
push rbx
mov  byte ptr [rbp-8], 1
xor  rax, rax
mov  qword ptr [rbp-16], rax
mov  rdi, rcx
mov  rsi, rdx
mov  r12, r8
mov  r13, r9
loop_start:
mov  rax, qword ptr [rbp-16]
cmp  rax, rsi
jae  loop_end
mov  rcx, rax
shl  rcx, 5
lea  rbx, [rdi + rcx]
mov  edx, dword ptr [r12]
inc  edx
mov  dword ptr [r12], edx
mov  r8d, dword ptr [r13]
dec  r8d
mov  dword ptr [r13], r8d
mov  ecx, edx
mov  edx, r8d
mov  r8,  rbx
call print_status
lea  rdx, [rsp]
mov  rcx, rbx
call fs_exists_with_ec
test al, al
jz   try_true
lea  rdx, [rsp]
mov  rcx, rbx
call fs_remove_with_ec
jmp  try_after
try_true:
mov  al, 1
try_after:
mov  cl, byte ptr [rbp-8]
and  cl, al
mov  byte ptr [rbp-8], cl
mov  rax, qword ptr [rbp-16]
inc  rax
mov  qword ptr [rbp-16], rax
jmp  loop_start
loop_end:
movzx eax, byte ptr [rbp-8]
pop  rbx
pop  rdi
pop  rsi
mov  rsp, rbp
pop  rbp
ret
cleanFiles ENDP
</code></pre>

<h2>downloadDataset (assembly snippet)</h2>
<pre class="code-block language-asm"><code>
EXTERN g_file_list : QWORD
EXTERN cleanFiles  : PROC
downloadDataset PROC
push rbp
mov  rbp, rsp
sub  rsp, 32
mov  dword ptr [rbp-4], 100
mov  dword ptr [rbp-8], -100
lea  rcx, g_file_list
mov  rdx, 4
lea  r8,  [rbp-4]
lea  r9,  [rbp-8]
call cleanFiles
mov  eax, 1
mov  rsp, rbp
pop  rbp
ret
downloadDataset ENDP
</code></pre>

<h2>Future work</h2>
<ul>
<li>Preprocess historical data (sources provide per-symbol downloads).</li>
<li>Download per-symbol files, aggregate into a combined dataset, then play back trades from the combined file.</li>
<li>Support single-file storage and aggregation at daily, weekly, monthly, and yearly granularities.</li>
</ul>

<p>Target Windows stack (examples): WinRT, WinUI 3, DirectX, Vulkan, OpenGL, other graphics elements.</p>

<h2>Notes</h2>
<p>Alpha Vantage API demo key used for examples. Example download URL and CSV format shown below.</p>

<pre class="code-block"><code>https://www.alphavantage.co/query?function=TIME_SERIES_INTRADAY&amp;symbol=AMZN&amp;interval=1min&amp;outputsize=full&amp;datatype=csv&amp;apikey=demo
// Filename: /AlphaVantage/Downloads/2018-09-07/intraday_1min_AMZN.csv
// Format: timestamp, open, high, low, close, volume
// 2018-09-07 15:59:00,1953.0500,1953.2800,1952.2200,1953.1000,63837</code></pre>

<h3>Yahoo Finance notes</h3>
<p>Yahoo Finance imposes limits: typically up to 7–8 days per request and practical limits on historical ranges. Confirm current API behavior before bulk downloads.</p>

<pre class="code-block"><code>https://query1.finance.yahoo.com/v8/finance/chart/AMZN?interval=1m&amp;range=7d
https://query1.finance.yahoo.com/v8/finance/chart/IBM?interval=1m&amp;range=7d
https://query1.finance.yahoo.com/v8/finance/chart/INTC?interval=1m&amp;range=7d
https://query1.finance.yahoo.com/v8/finance/chart/MSFT?interval=1m&amp;range=7d
https://query1.finance.yahoo.com/v8/finance/chart/NVDA?interval=1m&amp;range=7d</code></pre>

<p style="margin-top:12px;font-size:13px;color:#0b1220;">Copy this entire &lt;div&gt; block and save as <code>README.html</code> or paste into your documentation where the surrounding style wrapper is required. If you want more aggressive, client-side syntax highlighting (colors for every token), I can wrap keywords and literals more thoroughly or provide a ready-to-paste Prism/Highlight.js snippet inside this same wrapper.</p>
</div>
</div>
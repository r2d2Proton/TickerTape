<div style="border-left: 4px solid #007acc; padding: 0px; background-color: #f0f8ff;">
<div style="padding:16px; font-family: system-ui, -apple-system, 'Segoe UI', Roboto, Arial, sans-serif; color:#0b1220; line-height:1.45;">
<h1 style="margin:0 0 8px 0;">TickerTape</h1>
<p style="margin:0 0 12px 0;">Quick demonstration mixing modern C++ elements/containers (tuple, map, vector) with <code>struct</code> and showing how a small utility cleans files and reports status. Includes C++ examples and illustrative x64 assembly snippets.</p>

<hr style="border:none;border-top:1px solid rgba(11,18,32,0.06);margin:12px 0;">

<h2 style="margin:8px 0;">C++ Lambda Capture Block</h2>
<p style="margin:0 0 8px 0;">Capture variables inside square brackets <code>[]</code>. Example:</p>
<pre style="background:#ffffff;padding:10px;border-radius:4px;overflow:auto;"><code>auto fn = [x, &y](int z) {
return x + y + z;
};</code></pre>

<h2 style="margin:12px 0 6px 0;">Example Output (value parameters)</h2>
<pre style="background:#ffffff;padding:10px;border-radius:4px;overflow:auto;"><code>in=1 out=-1 filename=C:\Users\Jerry\Downloads\SymbolsURLs.txt
in=2 out=-2 filename=C:\Users\Jerry\Downloads\StocksURLs.txt
in=3 out=-3 filename=C:\Users\Jerry\Downloads\CombinedStocks.csv
in=4 out=-4 filename=C:\Users\Jerry\Downloads\CombinedStocks.csv</code></pre>

<h2 style="margin:12px 0 6px 0;">cleanFiles — value parameters</h2>
<p style="margin:0 0 8px 0;">Deletes a list of files and reports progress. Uses a lambda <code>tryDelete</code> that updates local counters passed by value.</p>
<pre style="background:#ffffff;padding:10px;border-radius:4px;overflow:auto;"><code>static bool cleanFiles(std::initializer_list&lt;string&gt; filenames, int in = 0, int out = 0)
{
namespace fs = std::filesystem;
auto tryDelete = [&in, &out](const string& filename) -> bool
{
in++;
--out;
cout &lt;&lt; "in=" &lt;&lt; in &lt;&lt; " out=" &lt;&lt; out &lt;&lt; " filename=" &lt;&lt; filename &lt;&lt; endl;
std::error_code ec;
if (fs::exists(filename, ec))
{
fs::remove(filename, ec);
return !ec;
}
return true;
};
bool lOK = true;
for (const auto& filename : filenames)
lOK &= tryDelete(filename);
return lOK;
}</code></pre>

<h3 style="margin:12px 0 6px 0;">Usage (value example)</h3>
<pre style="background:#ffffff;padding:10px;border-radius:4px;overflow:auto;"><code>bool downloadDataset(
Stock& stocks,
map&lt;string, string&gt;&amp; symbols,
_TICKER_TAPE_ARGS&amp; args
)
{
if (args.bCleanApp)
{
cout &lt;&lt; "Deleting old files...";
cleanFiles({
fullpathSymbolsURLsFilename,
fullpathStocksURLsFilename,
fullpathCombinedStocksFilename,
fullpathParseStocksFilename
});
cout &lt;&lt; "done." &lt;&lt; endl;
}
return true;
}</code></pre>

<hr style="border:none;border-top:1px solid rgba(11,18,32,0.06);margin:12px 0;">

<h2 style="margin:8px 0;">x64 Assembly (illustrative) — value version</h2>
<p style="margin:0 0 8px 0;"><strong>Notes:</strong> MSVC x64 calling convention: <code>RCX</code>=pointer, <code>RDX</code>=count. Shadow space reserved. This assembly is illustrative — adjust <code>sizeof(std::string)</code> per your STL implementation.</p>
<pre style="background:#ffffff;padding:10px;border-radius:4px;overflow:auto;"><code>; bool cleanFiles(std::initializer_list&lt;std::string&gt; filenames, int in = 0, int out = 0)
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
cleanFiles ENDP</code></pre>

<hr style="border:none;border-top:1px solid rgba(11,18,32,0.06);margin:12px 0;">

<h2 style="margin:8px 0;">Reference-parameter variant</h2>
<p style="margin:0 0 8px 0;">Pass counters by reference to preserve and observe caller-side changes. Example output and code below.</p>

<h3 style="margin:8px 0 6px 0;">Example output (reference)</h3>
<pre style="background:#ffffff;padding:10px;border-radius:4px;overflow:auto;"><code>in=101 out=-101 filename=C:\Users\Jerry\Downloads\SymbolsURLs.txt
in=102 out=-102 filename=C:\Users\Jerry\Downloads\StocksURLs.txt
in=103 out=-103 filename=C:\Users\Jerry\Downloads\CombinedStocks.csv
in=104 out=-104 filename=C:\Users\Jerry\Downloads\CombinedStocks.csv</code></pre>

<h3 style="margin:8px 0 6px 0;">cleanFiles — reference parameters</h3>
<pre style="background:#ffffff;padding:10px;border-radius:4px;overflow:auto;"><code>static bool cleanFiles(std::initializer_list&lt;string&gt; filenames, int&amp; in, int&amp; out)
{
namespace fs = std::filesystem;
auto tryDelete = [&in, &out](const string& filename) -> bool
{
in++;
--out;
cout &lt;&lt; "in=" &lt;&lt; in &lt;&lt; " out=" &lt;&lt; out &lt;&lt; " filename=" &lt;&lt; filename &lt;&lt; endl;
std::error_code ec;
if (fs::exists(filename, ec))
{
fs::remove(filename, ec);
return !ec;
}
return true;
};
bool lOK = true;
for (const auto& filename : filenames)
lOK &= tryDelete(filename);
return lOK;
}</code></pre>

<h3 style="margin:8px 0 6px 0;">Usage (reference example)</h3>
<pre style="background:#ffffff;padding:10px;border-radius:4px;overflow:auto;"><code>bool downloadDataset(
Stock& stocks,
map&lt;string, string&gt;&amp; symbols,
_TICKER_TAPE_ARGS&amp; args
)
{
int in = 100;
int out = -100;
if (args.bCleanApp)
{
cout &lt;&lt; "Deleting old files...";
cleanFiles({
fullpathSymbolsURLsFilename,
fullpathStocksURLsFilename,
fullpathCombinedStocksFilename,
fullpathParseStocksFilename
},
in,
out
);
cout &lt;&lt; "done" &lt;&lt; endl;
}
return true;
}</code></pre>

<h2 style="margin:12px 0 6px 0;">x64 Assembly Notes (reference)</h2>
<p style="margin:0 0 8px 0;"><strong>Preserved behavior</strong></p>
<ul style="margin:0 0 12px 20px;">
<li>Reference-based updates: <code>in</code> and <code>out</code> mutated via pointers (R8, R9).</li>
<li><code>initializer_list</code> ABI: pointer + count in RCX/RDX.</li>
<li>Lambda inlining: captured refs are pointers; no closure object required.</li>
<li>Filesystem calls abstracted to helper routines for <code>exists</code> and <code>remove</code>.</li>
<li>Proper stack frame and nonvolatile register preservation illustrated.</li>
</ul>

<pre style="background:#ffffff;padding:10px;border-radius:4px;overflow:auto;"><code>; bool cleanFiles(std::initializer_list&lt;string&gt; filenames, int&amp; in, int&amp; out)
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
cleanFiles ENDP</code></pre>

<h2 style="margin:12px 0 6px 0;">downloadDataset (assembly snippet)</h2>
<pre style="background:#ffffff;padding:10px;border-radius:4px;overflow:auto;"><code>EXTERN g_file_list : QWORD
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
downloadDataset ENDP</code></pre>

<h2 style="margin:12px 0 6px 0;">Future work</h2>
<p style="margin:0 0 8px 0;">Planned: animate stock trades in 3D. High-level steps:</p>
<ul style="margin:0 0 12px 20px;">
<li>Preprocess historical data (sources provide per-symbol downloads).</li>
<li>Download per-symbol files, aggregate into a combined dataset, then play back trades from the combined file.</li>
<li>Support single-file storage and aggregation at daily, weekly, monthly, and yearly granularities.</li>
</ul>

<p style="margin:0 0 8px 0;">Target Windows stack (examples):</p>
<ul style="margin:0 0 12px 20px;">
<li>WinRT</li>
<li>WinUI 3</li>
<li>DirectX</li>
<li>Vulkan</li>
<li>OpenGL</li>
<li>Other graphics elements</li>
</ul>

<h2 style="margin:12px 0 6px 0;">Notes</h2>
<p style="margin:0 0 8px 0;">Alpha Vantage API demo key was used for examples. Example download URL and CSV format shown below.</p>

<h3 style="margin:8px 0 6px 0;">Alpha Vantage example</h3>
<pre style="background:#ffffff;padding:10px;border-radius:4px;overflow:auto;"><code>https://www.alphavantage.co/query?function=TIME_SERIES_INTRADAY&amp;symbol=AMZN&amp;interval=1min&amp;outputsize=full&amp;datatype=csv&amp;apikey=demo
// Filename: /AlphaVantage/Downloads/2018-09-07/intraday_1min_AMZN.csv
// Format: timestamp, open, high, low, close, volume
// 2018-09-07 15:59:00,1953.0500,1953.2800,1952.2200,1953.1000,63837
// 2018-09-07 15:58:00,1953.1700,1953.7800,1952.9009,1952.9100,33356</code></pre>

<h3 style="margin:8px 0 6px 0;">Yahoo Finance notes</h3>
<p style="margin:0 0 8px 0;">Yahoo Finance imposes limits: typically up to 7–8 days per request and practical limits on historical ranges. Confirm current API behavior before bulk downloads.</p>

<h3 style="margin:8px 0 6px 0;">Yahoo Finance example URLs</h3>
<pre style="background:#ffffff;padding:10px;border-radius:4px;overflow:auto;"><code>https://query1.finance.yahoo.com/v8/finance/chart/AMZN?interval=1m&amp;range=7d
https://query1.finance.yahoo.com/v8/finance/chart/IBM?interval=1m&amp;range=7d
https://query1.finance.yahoo.com/v8/finance/chart/INTC?interval=1m&amp;range=7d
https://query1.finance.yahoo.com/v8/finance/chart/MSFT?interval=1m&amp;range=7d
https://query1.finance.yahoo.com/v8/finance/chart/NVDA?interval=1m&amp;range=7d</code></pre>

</div>
</div>
<!-- TickerTape README - fully inline HTML -->
<div style="border-left: 4px solid #007acc; padding: 0px; background-color: #f0f8ff;">
  <h1>TickerTape</h1>
  <p>Quick demonstration of mixing modern C++ elements/containers (tuple/map/vector) and struct</p>

  <h3>Test Extremes of Languages</h3>
  <p>The C++ Lambda Capture Block is one of them.</p>
  <pre><code>auto fn = [x, &y](int z) {
    return x + y + z;
};</code></pre>

  <h4>Output</h4>
  <pre><code>in=1 out=-1 filename=C:\Users\Jerry\Downloads\SymbolsURLs.txt
in=2 out=-2 filename=C:\Users\Jerry\Downloads\StocksURLs.txt
in=3 out=-3 filename=C:\Users\Jerry\Downloads\CombinedStocks.csv
in=4 out=-4 filename=C:\Users\Jerry\Downloads\CombinedStocks.csv</code></pre>

  <h4>Code: cleanFiles (value parameters)</h4>
  <pre><code>static bool cleanFiles(std::initializer_list<string> filenames, int in = 0, int out = 0)
{
    namespace fs = std::filesystem;
    auto tryDelete = [&in, &out](const string& filename) -> bool
    {
        in++;
        --out;
        cout << "in=" << in << " out=" << out << " filename=" << filename << endl;
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

  <h4>Usage (snippet)</h4>
  <pre><code>bool downloadDataset
(
    Stock& stocks,
    map<string, string>& symbols,
    _TICKER_TAPE_ARGS& args
)
{
    if (args.bCleanApp)
    {
        cout << "Deleting old files...";
        cleanFiles
        ({
            fullpathSymbolsURLsFilename,
            fullpathStocksURLsFilename,
            fullpathCombinedStocksFilename,
            fullpathParseStocksFilename
        });
        cout << "done." << endl;
    }
    return true;
}</code></pre>

  <h3>x64 Assembly for the cleanFiles function</h3>
  <p><strong>Notes</strong></p>
  <ul>
    <li>Stack frame: push rbp / mov rbp, rsp / sub rsp, 32 → standard frame + shadow space</li>
    <li>No caller-saved registers forgotten (we use rbx, rdi, rsi — in production save/restore any non-volatile registers you touch).</li>
  </ul>

  <pre><code>; bool cleanFiles(std::initializer_list<std::string> filenames, int in = 0, int out = 0)
; On entry (MSVC x64):
;   RCX = filenames._First  (std::string*)
;   RDX = filenames._Count  (size_t)
EXTERN fs_exists_with_ec : PROC    ; bool fs_exists_with_ec(std::string* filename, std::error_code* ec);
EXTERN fs_remove_with_ec : PROC    ; bool fs_remove_with_ec(std::string* filename, std::error_code* ec);
EXTERN print_status      : PROC    ; void print_status(int in, int out, std::string* filename);
cleanFiles PROC
    ; prolog: standard frame, 32 bytes of shadow space
    push rbp
    mov  rbp, rsp
    sub  rsp, 32                 ; shadow space
    ;--------------------------------------------
    ; Local layout (relative to RBP, negative):
    ; [rbp-4]   = lOK (bool, 4 bytes enough)
    ; [rbp-8]   = padding
    ; [rbp-16]  = index i (size_t)
    ; [rbp-24]  = in (int, stored in 8 bytes)
    ; [rbp-32]  = out (int, stored in 8 bytes)
    ;--------------------------------------------
    ; in = 0; out = 0;
    xor  eax, eax
    mov  qword ptr [rbp-24], rax      ; in = 0
    mov  qword ptr [rbp-32], rax      ; out = 0
    ; lOK = true;
    mov  dword ptr [rbp-4], 1
    ; i = 0;
    xor  r8, r8
    mov  qword ptr [rbp-16], r8
    ; RCX = filenames._First (std::string*)
    ; RDX = filenames._Count (size_t)
    ; We'll keep:
    ;   RDI = base pointer to first filename (std::string*)
    ;   RSI = count
    mov  rdi, rcx
    mov  rsi, rdx
; for (size_t i = 0; i &lt; count; ++i)
for_loop_start:
    mov  rax, qword ptr [rbp-16] ; rax = i
    cmp  rax, rsi
    jae  for_loop_end
    ; const std::string&amp; filename = filenames[i];
    ; filename_ptr = base + i * sizeof(std::string)
    ; You must use the actual sizeof(std::string) from your implementation.
    ; For illustration, assume 32 bytes:
    ;   filename_ptr = rdi + rax * 32
    mov  rcx, rax
    shl  rcx, 5                   ; rcx = i * 32 (2^5)
    lea  rbx, [rdi + rcx]         ; rbx = &filenames[i] (std::string*)
    ; --- tryDelete(filename) inline ---
    ; in++;
    mov  rdx, qword ptr [rbp-24]  ; load in
    inc  rdx
    mov  qword ptr [rbp-24], rdx
    ; --out;
    mov  r8,  qword ptr [rbp-32]  ; load out
    dec  r8
    mov  qword ptr [rbp-32], r8
    ; cout &lt;&lt; "in=..." ... (we'll call a helper)
    ; print_status(in, out, &amp;filename);
    ;   RCX = in
    ;   RDX = out
    ;   R8  = filename pointer
    mov  ecx, dword ptr [rbp-24]  ; in
    mov  edx, dword ptr [rbp-32]  ; out
    mov  r8,  rbx                 ; &amp;filename
    call print_status
    ; std::error_code ec; (we just reserve stack space via shadow space area)
    ; We'll use 16 bytes in shadow space as temporary for ec:
    ;   [rbp-48] etc. is possible, but we already reserved exactly 32.
    ; For clarity, let's just reuse shadow space as a spill for ec object:
    ;   at [rsp] while calls are made
    ; if (fs::exists(filename, ec))
    ; call: bool fs_exists_with_ec(std::string* filename, std::error_code* ec);
    ; &amp;ec in RDX, filename in RCX
    lea  rdx, [rsp]               ; ec at shadow space
    mov  rcx, rbx                 ; &amp;filename
    call fs_exists_with_ec
    test al, al
    jz   tryDelete_return_true    ; if not exists, return true
    ; fs::remove(filename, ec)
    ; bool fs_remove_with_ec(std::string* filename, std::error_code* ec);
    lea  rdx, [rsp]               ; &amp;ec
    mov  rcx, rbx                 ; &amp;filename
    call fs_remove_with_ec
    ; return !ec;  (our helper already returns !ec as bool in AL)
    ; so AL is tryDelete result
    jmp  tryDelete_after
tryDelete_return_true:
    mov  al, 1                    ; true
tryDelete_after:
    ; lOK &amp;= tryDelete_result;
    mov  ecx, dword ptr [rbp-4]   ; lOK
    and  cl, al                   ; lOK &amp;= result
    mov  dword ptr [rbp-4], ecx
    ; ++i;
    mov  rax, qword ptr [rbp-16]
    inc  rax
    mov  qword ptr [rbp-16], rax
    jmp  for_loop_start
for_loop_end:
    ; return lOK;
    mov  eax, dword ptr [rbp-4]   ; lOK
    and  eax, 1                   ; normalize to 0/1
    ; epilog
    mov  rsp, rbp
    pop  rbp
    ret
cleanFiles ENDP</code></pre>

  <h4>And using by reference with different initial values +100,-100 to differentiate the output</h4>
  <pre><code>Output
in=101 out=-101 filename=C:\Users\Jerry\Downloads\SymbolsURLs.txt
in=102 out=-102 filename=C:\Users\Jerry\Downloads\StocksURLs.txt
in=103 out=-103 filename=C:\Users\Jerry\Downloads\CombinedStocks.csv
in=104 out=-104 filename=C:\Users\Jerry\Downloads\CombinedStocks.csv</code></pre>

  <h4>Code: cleanFiles (reference parameters)</h4>
  <pre><code>static bool cleanFiles(std::initializer_list<string> filenames, int& in, int& out)
{
    namespace fs = std::filesystem;
    auto tryDelete = [&in, &out](const string& filename) -> bool
    {
        in++;
        --out;
        cout << "in=" << in << " out=" << out << " filename=" << filename << endl;
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

  <h4>Usage (reference example)</h4>
  <pre><code>bool downloadDataset
(
    Stock& stocks,
    map<string, string>& symbols,
    _TICKER_TAPE_ARGS& args
)
{
    int in = 100;
    int out = -100;
    if (args.bCleanApp)
    {
        cout << "Deleting old files...";
        cleanFiles
        (   {
                fullpathSymbolsURLsFilename,
                fullpathStocksURLsFilename,
                fullpathCombinedStocksFilename,
                fullpathParseStocksFilename
            },
            in,
            out
        );
        cout << "done." << endl;
    }
    return true;
}</code></pre>

  <h3>x64 Assembly Notes (reference version)</h3>
  <p><strong>What this assembly preserves</strong></p>
  <ul>
    <li>reference-based updates: in and out are mutated through pointers (R8, R9)</li>
    <li>initializer_list ABI: pointer + count in RCX/RDX</li>
    <li>lambda inlining: no closure object needed; captured refs are just pointers</li>
    <li>filesystem calls abstracted: external helpers for exists and remove</li>
    <li>proper stack frame: push rbp / mov rbp,rsp / sub rsp,32</li>
    <li>nonvolatile register preservation: RBX, RDI, RSI saved/restored</li>
    <li>looping over std::string array: pointer arithmetic with sizeof(std::string)=32</li>
  </ul>

  <pre><code>; bool cleanFiles(std::initializer_list<string> filenames, int&amp; in, int&amp; out)
; RCX = filenames._First (std::string*)
; RDX = filenames._Count (size_t)
; R8  = &amp;in
; R9  = &amp;out
EXTERN fs_exists_with_ec : PROC
EXTERN fs_remove_with_ec : PROC
EXTERN print_status      : PROC     ; printf-style helper
cleanFiles PROC
    push rbp
    mov  rbp, rsp
    sub  rsp, 32                    ; shadow space
    ; Save nonvolatile registers we use
    push rsi
    push rdi
    push rbx
    ; locals:
    ; [rbp-8]  = lOK (bool)
    ; [rbp-16] = index i
    mov  byte ptr [rbp-8], 1        ; lOK = true
    xor  rax, rax
    mov  qword ptr [rbp-16], rax    ; i = 0
    mov  rdi, rcx                   ; base pointer to std::string[]
    mov  rsi, rdx                   ; count
    mov  r12, r8                    ; &amp;in
    mov  r13, r9                    ; &amp;out
loop_start:
    mov  rax, qword ptr [rbp-16]
    cmp  rax, rsi
    jae  loop_end
    ; filename = &amp;filenames[i]
    mov  rcx, rax
    shl  rcx, 5                     ; sizeof(std::string)=32
    lea  rbx, [rdi + rcx]           ; rbx = filename*
    ; --- tryDelete(filename) inline ---
    ; ++(*in)
    mov  edx, dword ptr [r12]
    inc  edx
    mov  dword ptr [r12], edx
    ; --(*out)
    mov  r8d, dword ptr [r13]
    dec  r8d
    mov  dword ptr [r13], r8d
    ; print_status(in, out, filename)
    mov  ecx, edx                   ; in
    mov  edx, r8d                   ; out
    mov  r8,  rbx                   ; filename
    call print_status
    ; exists?
    lea  rdx, [rsp]                 ; error_code on shadow space
    mov  rcx, rbx
    call fs_exists_with_ec
    test al, al
    jz   try_true                   ; if not exists → true
    ; remove
    lea  rdx, [rsp]
    mov  rcx, rbx
    call fs_remove_with_ec          ; returns !ec in AL
    jmp  try_after
try_true:
    mov  al, 1
try_after:
    ; lOK &amp;= AL
    mov  cl, byte ptr [rbp-8]
    and  cl, al
    mov  byte ptr [rbp-8], cl
    ; ++i
    mov  rax, qword ptr [rbp-16]
    inc  rax
    mov  qword ptr [rbp-16], rax
    jmp  loop_start
loop_end:
    movzx eax, byte ptr [rbp-8]
    ; restore nonvolatile regs
    pop  rbx
    pop  rdi
    pop  rsi
    mov  rsp, rbp
    pop  rbp
    ret
cleanFiles ENDP</code></pre>

  <h3>downloadDataset (assembly snippet)</h3>
  <pre><code>EXTERN g_file_list : QWORD        ; array of 4 std::string
EXTERN cleanFiles  : PROC
downloadDataset PROC
    push rbp
    mov  rbp, rsp
    sub  rsp, 32
    ; locals: in, out
    mov  dword ptr [rbp-4], 100     ; in = 100
    mov  dword ptr [rbp-8], -100    ; out = -100
    ; Build initializer_list<string> in registers:
    lea  rcx, g_file_list           ; RCX = first
    mov  rdx, 4                     ; RDX = count
    lea  r8,  [rbp-4]               ; R8  = &amp;in
    lea  r9,  [rbp-8]               ; R9  = &amp;out
    call cleanFiles
    mov  eax, 1                     ; return true
    mov  rsp, rbp
    pop  rbp
    ret
downloadDataset ENDP</code></pre>

  <h3>Future work</h3>
  <p>Time permitting will animate stock trades in 3D:</p>
  <ul>
    <li>requires preprocessing of historical data since the sources this project uses provides downloads of a single, specific, stock symbol</li>
    <li>the algorithm will basically download stocks based on a time frame and stocks symbols of interest into individual files then aggregate the downloaded stocks into a single file then play back the stocks from the combined file</li>
    <li>focusing on Single file, Daily, Weekly, Monthly, and Yearly for local storage</li>
  </ul>

  <p>Windows based with a combination of:</p>
  <ul>
    <li>WinRT</li>
    <li>WinUI 3</li>
    <li>DirectX</li>
    <li>Vulkan</li>
    <li>OpenGL</li>
    <li>other graphics elements</li>
  </ul>

  <h3>Notes</h3>
  <p>Alpha Vantage apiKey demo used to work</p>
  <p>Alpha Vantage download URL:</p>
  <pre><code>https://www.alphavantage.co/query?function=TIME_SERIES_INTRADAY&amp;symbol=AMZN&amp;interval=1min&amp;outputsize=full&amp;datatype=csv&amp;apikey=demo
// Filename: /AlphaVantage/Downloads/2018-09-07/intraday_1min_AMZN.csv
//
// Format: timestamp, open, high, low, close, volume
//
// 2018-09-07 15:59:00,1953.0500,1953.2800,1952.2200,1953.1000,63837
// 2018-09-07 15:58:00,1953.1700,1953.7800,1952.9009,1952.9100,33356
// :
// 2018-09-07 09:31:00,1944.4000,1949.3550,1944.4000,1949.3550,52008
// 2018-09-07 09:30:00,1937.9301,1944.8199,1937.8101,1944.0699,215066</code></pre>

  <p>Yahoo Finance has other restrictions</p>
  <p>maximum of 8 days can be downloaded at a time; no more than the past 30 days (if read correctly)</p>
  <pre><code>// Yahoo Finance download URL examples:
https://query1.finance.yahoo.com/v8/finance/chart/AMZN?interval=1m&amp;range=7d
https://query1.finance.yahoo.com/v8/finance/chart/IBM?interval=1m&amp;range=7d
https://query1.finance.yahoo.com/v8/finance/chart/INTC?interval=1m&amp;range=7d
https://query1.finance.yahoo.com/v8/finance/chart/MSFT?interval=1m&amp;range=7d
https://query1.finance.yahoo.com/v8/finance/chart/NVDA?interval=1m&amp;range=7d</code></pre>
</div>

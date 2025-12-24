# TickerTape

Quick demonstration mixing modern C++ elements/containers (tuple, map, vector) with `struct`. Includes C++ examples and illustrative x64 assembly snippets. Code blocks use standard fenced Markdown so they render correctly on GitHub and in most viewers.

---

## README Formatting Preference

This project prefers README.md content to use standard Markdown fenced code blocks with explicit languages for syntax highlighting:

- Use triple-backtick fences with `cpp` for C++ examples.
- Use triple-backtick fences with `asm` for x64 assembly examples.
- Avoid inline HTML with span-based tokenization in README.md so GitHub and other renderers display code correctly.

## C++ Lambda Capture Block

Capture variables inside square brackets `[]`. Example:

```cpp
auto fn = [x, &y](int z) { return x + y + z; };
```

## Example Output (value parameters)

```
in=1  out=-1  filename=C:\Users\Jerry\Downloads\SymbolsURLs.txt
in=2  out=-2  filename=C:\Users\Jerry\Downloads\StocksURLs.txt
in=3  out=-3  filename=C:\Users\Jerry\Downloads\CombinedStocks.csv
in=4  out=-4  filename=C:\Users\Jerry\Downloads\CombinedStocks.csv
```

## cleanFiles — value parameters

Deletes a list of files and reports progress. Lambda `tryDelete` updates local counters passed by value.

```cpp
static bool cleanFiles(std::initializer_list<string> filenames, int in = 0, int out = 0)
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
}
```

### Usage (value example)

```cpp
bool downloadDataset(
    Stock& stocks,
    map<string, string>& symbols,
    _TICKER_TAPE_ARGS& args
)
{
    if (args.bCleanApp)
    {
        cout << "Deleting old files...";
        cleanFiles({
            fullpathSymbolsURLsFilename,
            fullpathStocksURLsFilename,
            fullpathCombinedStocksFilename,
            fullpathParseStocksFilename
        });
        cout << "done." << endl;
    }
    return true;
}
```

---

## x64 Assembly (illustrative) — value version

Notes: MSVC x64 calling convention: `RCX`=pointer, `RDX`=count. Shadow space reserved. Adjust `sizeof(std::string)` per your STL implementation.

```asm
; bool cleanFiles(std::initializer_list<std::string> filenames, int in = 0, int out = 0)
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
```

---

## Reference-parameter variant

Pass counters by reference to preserve and observe caller-side changes.

### Example output (reference)

```
in=101  out=-101  filename=C:\Users\Jerry\Downloads\SymbolsURLs.txt
in=102  out=-102  filename=C:\Users\Jerry\Downloads\StocksURLs.txt
in=103  out=-103  filename=C:\Users\Jerry\Downloads\CombinedStocks.csv
in=104  out=-104  filename=C:\Users\Jerry\Downloads\CombinedStocks.csv
```

### cleanFiles — reference parameters

```cpp
static bool cleanFiles(std::initializer_list<string> filenames, int& in, int& out)
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
}
```

### Usage (reference example)

```cpp
bool downloadDataset(
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
        cleanFiles({
            fullpathSymbolsURLsFilename,
            fullpathStocksURLsFilename,
            fullpathCombinedStocksFilename,
            fullpathParseStocksFilename
        },
        in,
        out
        );
        cout << "done" << endl;
    }
    return true;
}
```

## x64 Assembly Notes (reference)

**Preserved behavior**

- Reference-based updates: `in` and `out` mutated via pointers (R8, R9).
- `initializer_list` ABI: pointer + count in RCX/RDX.
- Lambda inlining: captured refs are pointers; no closure object required.
- Filesystem calls abstracted to helper routines for `exists` and `remove`.
- Proper stack frame and nonvolatile register preservation illustrated.

```asm
; bool cleanFiles(std::initializer_list<string> filenames, int& in, int& out)
; RCX = filenames._First (std::string*)
; RDX = filenames._Count (size_t)
; R8  = &in
; R9  = &out
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
```

### downloadDataset (assembly snippet)

```asm
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
```

## Future work

- Preprocess historical data (sources provide per-symbol downloads).
- Download per-symbol files, aggregate into a combined dataset, then play back trades from the combined file.
- Support single-file storage and aggregation at daily, weekly, monthly, and yearly granularities.

Target Windows stack (examples): WinRT, WinUI 3, DirectX, Vulkan, OpenGL, other graphics elements.

## Notes

Alpha Vantage API demo key used for examples. Example download URL and CSV format shown below.

```
https://www.alphavantage.co/query?function=TIME_SERIES_INTRADAY&symbol=AMZN&interval=1min&outputsize=full&datatype=csv&apikey=demo
// Filename: /AlphaVantage/Downloads/2018-09-07/intraday_1min_AMZN.csv
// Format: timestamp, open, high, low, close, volume
// 2018-09-07 15:59:00,1953.0500,1953.2800,1952.2200,1953.1000,63837
```

### Yahoo Finance notes

Yahoo Finance imposes limits: typically up to 7–8 days per request and practical limits on historical ranges. Confirm current API behavior before bulk downloads.

```
https://query1.finance.yahoo.com/v8/finance/chart/AMZN?interval=1m&range=7d
https://query1.finance.yahoo.com/v8/finance/chart/IBM?interval=1m&range=7d
https://query1.finance.yahoo.com/v8/finance/chart/INTC?interval=1m&range=7d
https://query1.finance.yahoo.com/v8/finance/chart/MSFT?interval=1m&range=7d
https://query1.finance.yahoo.com/v8/finance/chart/NVDA?interval=1m&range=7d

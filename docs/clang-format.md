# clang-format

```bash
clang-format -style=google -dump-config | Out-File -Encoding ascii .clang-format

clang-format -i .\src\combat.cpp

Get-ChildItem .\src\ -Filter *.cpp | ForEach-Object { clang-format -i $_.FullName }

Get-ChildItem .\src\ -Filter *.h   | ForEach-Object { clang-format -i $_.FullName }
```

```c++
// clang-format off
...
// clang-format on
```
# clang-format

```bash
Get-ChildItem .\src\ -Filter *.cpp | ForEach-Object { clang-format -i $_.FullName }

Get-ChildItem .\src\ -Filter *.h   | ForEach-Object { clang-format -i $_.FullName }
```
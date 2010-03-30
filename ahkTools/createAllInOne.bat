set FILENAME=AllInOne.ahk
del %FILENAME%
for %%f in (singleFuncs\*.ahk) do type %%f >> %FILENAME%
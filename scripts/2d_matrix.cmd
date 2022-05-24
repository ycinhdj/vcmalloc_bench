for /L %%i in (21,1,24) do (
for /L %%j in (12,1,33) do (
for /L %%k in (1,1,10) do (

	.\..\out\vcmalloc_bench.exe %%i %%j %%k
)
)
)
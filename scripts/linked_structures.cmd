for /L %%i in (0,1,20) do (
for /L %%j in (12,1,28) do (
for /L %%t in (1,1,1) do (

	.\..\out\vcmalloc_bench.exe %%i %%j %%t
)
)
)
for /L %%i in (25,1,28) do (
for /L %%j in (12,1,16) do (
for /L %%k in (1,1,1) do (

	.\..\out\vcmalloc_bench.exe %%i %%j %%k
)
)
)
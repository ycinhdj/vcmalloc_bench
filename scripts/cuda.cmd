for /L %%i in (0,1,5) do (
for /L %%j in (2,1,5) do (
for /L %%t in (1,1,1) do (

	.\..\out\vcmalloc_bench_cuda.exe %%i %%j %%t
)
)
)
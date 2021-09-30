# Matrix Generator & Multiplier
## tools
`matrix_gen -h` - usage of matrix generator \
`g++ matrix.cpp` - create binary which calcultes **C = A x B**. \
matrix **A** should be in `matrixA.txt` and **B** should be in `matrixB.txt`

## run
```
time ./a.out > output.txt
```

## check
copy the content of matrixA.txt and matrixB.txt [here](https://matrix.reshish.com/multCalculation.php)

## correct run (single-threaded)
matrixA.txt
```
-80   989   430   -549  
541   847   669   875   
581   779   -310  489   
951   174   726   583   
948   -628  885   769   

```
matrixB.txt
```
583   -571  -391  -965  943   
-170  -24   -816  -811  -677  
103   876   53    154   951   
-740  60    121   -859  -380  

```
output:
```
$ ./a.out
```
```
Matrix A:
     -80     989     430    -549
     541     847     669     875
     581     779    -310     489
     951     174     726     583
     948    -628     885     769

Matrix B:
     583    -571    -391    -965     943
    -170     -24    -816    -811    -677
     103     876      53     154     951
    -740      60     121    -859    -380

Matrix C = AxB:
  235780  365684 -819383 -187068 -127443
 -407180  309305 -761351-1857581  240463
 -187497 -592667 -820096-1660225 -460130
  168211  123759 -404804-1447822 1247881
  181539  295164  281734 -929793 1868535
```
**Answer is correct.**
NPU = neural processor unit

https://en.wikichip.org/wiki/neural_processor

# TPU (ref 1)
## Neural network

We got input data (x1, x2, x3...etc). 
On multiplie les données par un poids wi et on accumule le résultat dans le neurone, puis on applique une fonction d'activation.

exemple : 
```
On a deux neurones y1 et y2, 3 data en inputs x1, x2 et x3.

y1 = w11*x1 + w12*x2 + w13*x3
y2 = w21*x1 + w22*x2 + w23*x3

Ensuite on applique une fonction d'activation :


y1 = f1(w11*x1 + w12*x2 + w13*x3)
y2 = f2(w21*x1 + w22*x2 + w23*x3)
```

Les fonctions d'activation peuvent être ReLU, sigmoid, tanh...etc

Du coup avec 3 inputs et deux neurones on a fait 6 multiplications et 6 additions. Pour simplifier on peut faire le calcul sous forme de matrice.
```
              x1   
w11 w12 w13 * x2  = w11*x1 + w12*x2 + w13*x3
              x3
```

## TPU architecture

Un TPU contient :
* Matrix multiplier unit (MXU)
* Unified buffer (UB)
* Activation unit (AU): Hardwired activation functions

Scalar processor are to slow to perform the kind of operation needed.
CPUs incorporate instruction set extensions such as **SSE** and **AVX** that express such vector operations.

The streaming multiprocessors (SMs) of GPUs are effectively vector processors, with many such SMs on a single GPU die.


![plot](vect.png)

In the case of the TPU, Google designed MXU as matrix processor that process hundreds of thousands of operations per cycle.

Pour faire ca le coeur du TPU utilise **systolic array**.

### **systolic array** :
```
One example of how a systolic array can work is in matrix multiplication. Suppose we have two matrices, A and B, with dimensions m x n and n x p, respectively. A systolic array with m processing elements arranged in a row can be used to calculate the matrix product C = A x B.

Initially, the first row of matrix A and the first column of matrix B are loaded into the systolic array. Each processing element multiplies its input values and passes the result to its right neighbor, which adds the result to its own accumulated sum. This process is repeated for each subsequent row of matrix A and column of matrix B, with the accumulated sums at the end of the row representing the corresponding element of the resulting matrix C.

By using a systolic array, the matrix multiplication operation can be performed in O(mnp) time, which is faster than the traditional O(n^3) time required for matrix multiplication using a sequential algorithm.
```

Pour les MXU, on va pas direct store le résultat dans un registre après le calcul de l'ALU, on va le réutiliser.
On a un chainage d'ALU pour effectuer des opérations.

# NPU architecture (ref 2)

NPU contains 8 PEs (processing engines). Each PE performs the computation of a neuron (multiplication, addition and sigmoid).

If a program segment is frequently executed and approximable, and if the inputs and outputs are well defined, then that segment can be accelerated by the NPU.

Pour utiliser un NPU, les programmeurs doivent manuellement anoté une portion de code qui satisfait les critères ci dessus.

![plot](npu.png)

# NPU vs TPU

There are some differences between NPUs and TPUs. One key difference is that TPUs are specifically designed to accelerate deep learning tasks, while NPUs can accelerate a broader range of machine learning algorithms. TPUs are also developed by Google and are only available on the Google Cloud Platform, while NPUs can be developed and used by any company or organization.

GPU perform SIMD instructions while NPU and TPU uses 2D SIMD.
# Références 

1. https://cloud.google.com/blog/products/ai-machine-learning/an-in-depth-look-at-googles-first-tensor-processing-unit-tpu?hl=en
2. A Survey of Accelerator Architectures for Deep Neural Networks
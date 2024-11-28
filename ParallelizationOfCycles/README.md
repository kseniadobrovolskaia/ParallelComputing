# Распараллеливание вложенных циклов

> [!NOTE]
> Основным заданием данной лабораторной работы является разработка и исследование параллельных программ, созданных на основе существующих заготовок последовательных программ. Полученные результаты требуется сравнить, а также изобразить графически для каждой из реализаций зависимость коэффициента ускорения программы от количества используемых исполнителей.


Лабораторная работа подразумевает выполнение распараллеливания при помощи двух разных технологий (с общей памятью **OpenMP** и с распределённой памятью **MPI**) трёх эталонных программ.


## Содержание

[1. Анализ последовательных программ](#1)



<a name="1"></a>
## Анализ последовательных программ

Вычислим следующие характеристики последовательных программ, чтобы понять как можно их распараллелить.

1. **Вычислим вектор направлений.**

2. **Вычислим вектор расстояний.**

3. **Определим тип зависимости и возможные варианты распараллеливания.**


```bash
$ batcat SequentialProgram1.c
```

```c
#include <stdio.h>
#include <stdlib.h>
#define ISIZE 1000
#define JSIZE 1000

int main(int argc, char **argv) {
  double a[ISIZE][JSIZE];
  int i, j;
  FILE *ff;
  //подготовительная часть – заполнение некими данными
  for (i = 0; i < ISIZE; i++) {
    for (j = 0; j < JSIZE; j++) {
      a[i][j] = 10 * i + j;
    }
  }
  // требуется обеспечить измерение времени работы данного цикла
  for (i = 0; i < ISIZE; i++) {
    for (j = 0; j < JSIZE; j++) {
      a[i][j] = sin(2 * a[i][j]);
    }
  }
  ff = fopen("result.txt", "w");
  for (i = 0; i < ISIZE; i++) {
    for (j = 0; j < JSIZE; j++) {
      fprintf(ff, "%f ", a[i][j]);
    }
    fprintf(ff, "\n");
  }
  fclose(ff);
}
```

1. 

2.

3. 



### MPI 


```bash
$ batcat SequentialProgramMPI.c
```

```c
int main(int argc, char **argv) {
  double a[ISIZE][JSIZE];
  int i, j;
  FILE *ff;
  for (i = 0; i < ISIZE; i++) {
    for (j = 0; j < JSIZE; j++) {
      a[i][j] = 10 * i + j;
    }
  }
  for (i = 2; i < ISIZE; i++) {
    for (j = 0; j < JSIZE - 3; j++) {
      a[i][j] = sin(5 * a[i - 2][j + 3]);
    }
  }
  ff = fopen("result.txt", "w");
  for (i = 0; i < ISIZE; i++) {
    for (j = 0; j < JSIZE; j++) {
      fprintf(ff, "%f ", a[i][j]);
    }
    fprintf(ff, "\n");
  }
  fclose(ff);
}

```


1. 

2.

3. 


### OpenMP



```bash
$ batcat SequentialProgramOpenMP.c
```

```c
double a[ISIZE][JSIZE], b[ISIZE][JSIZE];
int main(int argc, char **argv) {
  int i, j;
  FILE *ff;
  for (i = 0; i < ISIZE; i++) {
    for (j = 0; j < JSIZE; j++) {
      a[i][j] = 10 * i + j;
      b[i][j] = 0;
    }
  }
  ///начало измерения времени
  for (i = 0; i < ISIZE; i++) {
    for (j = 0; j < JSIZE; j++) {
      a[i][j] = sin(0.005 * a[i][j]);
    }
  }
  for (i = 5; i < ISIZE; i++) {
    for (j = 0; j < JSIZE - 2; j++) {
      b[i][j] = a[i - 5][j + 2] * 1.5;
    }
  }
  ///окончание измерения времени
  ff = fopen("result.txt", "w");
  for (i = 0; i < ISIZE; i++) {
    for (j = 0; j < JSIZE; j++) {
      fprintf(ff, "%f ", b[i][j]);
    }
    fprintf(ff, "\n");
  }
  fclose(ff);
}

```

1. 

2.

3. 




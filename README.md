## Необходимые зависимости и компиляция

Для компиляции надо установить на компьютере три зависимости: `glew glfw glm`.  
Потом копмилировать `imgui` зависимости и на последним этапе исполняемый.

### **На Arch Linux:**

```
> sudo pacman glew glfw glm
```

### **Для компиляции с g++**

```
> make build
> make release
```

# Гравитационная симуляция N тел

## Гравитационная задача N тел

Гравитационная задача N тел является классической проблемой небесной механики и гравитационной динамики Ньютона.

Она формулируется следующим образом:

В пустоте находится $N$ материальных точек, массы которых известны $\{m_i\}$. Пусть попарное взаимодействие точек подчинено закону тяготения Ньютона, и пусть силы гравитации аддитивны. Пусть известны начальные на момент времени $\,t=0\,$ положения и скорости каждой точки $\textbf{r}_i|_{t=0} = \textbf{v}_{i0},\, \textbf{v}_i|_{t=0} = \textbf{v}_{i0}.$ Требуется найти положения точек для всех последующих моментов времени.

https://ru.wikipedia.org/wiki/%D0%93%D1%80%D0%B0%D0%B2%D0%B8%D1%82%D0%B0%D1%86%D0%B8%D0%BE%D0%BD%D0%BD%D0%B0%D1%8F_%D0%B7%D0%B0%D0%B4%D0%B0%D1%87%D0%B0_N_%D1%82%D0%B5%D0%BB

## Числительные методы

Мы используем гравитационный потенциал для иллюстрации основной формы вычислений в моделировании всех пар N-тел. В следующих вычислениях мы используем жирный шрифт для обозначения векторов (обычно в 3D). Учитывая $N$ тел с начальным положением $\textbf{x}_i$ и скоростью $\textbf{v}_i$ для $ 1 \leqslant i \leqslant N$, вектор силы $\textbf{f}_{ij}$ на теле $i$, вызванной его гравитационным притяжением к телу $j$, задается следующим образом:

$$
\textbf{f}_{ij} = G\frac{m_im_j}{||\textbf{r}||^2}\cdot\frac{\textbf{r}_{ij}}{||\textbf{r}_{ij}||}
$$

где $m_i$ и $m_j$ $\text{---}$ массы тел $i$ и $j$, соответственно; $\textbf{r}_{ij} = \textbf{x}_j - \textbf{x}_i$ $\text{---}$ вектор от тела $i$ к телу $j\,$; $G$ $\text{---}$ гравитационная постоянная. Левый фактор, величина силы, пропорционален произведению масс и уменьшается с квадратом расстояния между телами $i$ и $j$. Правый фактор $\text{---}$ направление силы, единичный вектор от тела $i$ в направлении тела $j$ (поскольку гравитация является притягивающей силой).

Полная сила $\textbf{F}_i$, действующая на тело $i$ в результате его взаимодействия с другими $N - 1$ телами, получается суммированием всех взаимодействий:

$$
\textbf{F}_i = \underset{j \neq i}{\sum_{1 \leqslant j  \leqslant N}} \textbf{f}_i = Gm_i\underset{j \neq i}{\sum_{1 \leqslant j  \leqslant N}} \frac{m_j\textbf{r}_{ij}}{||\textbf{r}_{ij}||^3}
$$

По мере приближения тел друг к другу сила между ними неограниченно растет, что является нежелательной ситуацией для численного интегрирования. В астрофизических симуляциях столкновения между телами обычно исключаются; это разумно, если тела представляют собой галактики, которые могут проходить прямо друг через друга. Поэтому добавляется коэффициент смягчения $\varepsilon^2 > 0$, и знаменатель переписывается следующим образом:

$$
\textbf{F}_i \approx Gm_i\sum_{1 \leqslant j  \leqslant N} \frac{m_j\textbf{r}_{ij}}{\left(||\textbf{r}_{ij}||^2 + \varepsilon^2\right)^\frac{3}{2}}
$$

Обратите внимание, что условие $j \neq i$ больше не нужно в сумме, потому что $\textbf{f}_{ii} = 0$, когда $\varepsilon^2 > 0$. Фактор смягчения моделирует взаимодействие между двумя точечными массами Пламмера: массами, которые ведут себя так, как если бы они были сферическими галактиками (Aarseth 2003, Dyer and Ip 1993). По сути, фактор смягчения ограничивает величину силы между телами, что желательно для численного интегрирования состояния системы.

Для интегрирования по времени нам необходимо ускорение $\textbf{a}_i = \cfrac{\textbf{F}_i}{m_i}$ для обновления положения и скорости тела $i$, поэтому мы упрощаем вычисления до этого:

$$
\textbf{a}_i \approx G\sum_{1 \leqslant j  \leqslant N} \frac{m_j\textbf{r}_{ij}}{\left(||\textbf{r}_{ij}||^2 + \varepsilon^2\right)^\frac{3}{2}}
$$

Интегратор, используемый для обновления положений и скоростей, является интегратором скачка Верле (Verlet 1967), поскольку он применим к данной задаче и является вычислительно эффективным (имеет высокое отношение точности к вычислительным затратам). Выбор метода интегрирования в задачах N-тел обычно зависит от природы изучаемой системы. Интегратор включен в наши тайминги, но обсуждение его реализации опущено, поскольку его сложность составляет $O(N)$ и его стоимость становится незначительной с ростом $N$.

https://developer.nvidia.com/gpugems/gpugems3/part-v-physics-simulation/chapter-31-fast-n-body-simulation-cuda

## Интегратор Leap-frog

## Алгоритм Barnes-Hut

http://arborjs.org/docs/barnes-hut

### Моя реализация алгоритма на GPU

## Thread divergence

https://iss.oden.utexas.edu/Publications/Papers/burtscher11.pdf

## Недостатки

### Будущие оптимизации

## Примечания

https://github.com/salel/nbody/

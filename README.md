# graph-analytics - LeaderRank

Вычисление меры центральности **LeaderRank** на большом ориентированном графе,
который не помещается в оперативную память. Рёбра потоково читаются с диска: в RAM держится только вектор рангов `O(|V|)`, а вычисление
распараллелено по всем ядрам.

## Сборка

Нужен CMake $\ge$ 3.16 и компилятор с C++20. На Ubuntu 22.04:

```bash
sudo apt install -y build-essential cmake
mkdir build && cd build
cmake ..
make -j
```

Бинарь - `build/gra`. Другой компилятор при необходимости: `cmake -DCMAKE_CXX_COMPILER=g++ ..`

## Использование

Два шага (команды - из каталога `build/`, после сборки вы уже там): препроцессинг
CSV в бинарный вид, затем вычисление.

```bash
./gra preprocess --input ../data/edges.csv --output out [--shards 64]
./gra run --input out --output out/leader_rank.csv [--threads N] [--max-iter 1000] [--tol 1e-6]
```

- `--shards` - число бинарных шардов (по умолчанию 64)
- `--threads` - число потоков (по умолчанию: все ядра)
- `--max-iter` - максимум итераций (по умолчанию 1000)
- `--tol` - порог сходимости по относительной норме `‖Δ‖₁ / N` (по умолчанию 1e-6)

## Формат данных

Вход - CSV с шапкой `from,to`, две колонки `int32`. Рёбра ориентированные и
невзвешенные:

```
from,to
1,2
2,3
3,1
```

Выход - CSV с шапкой `vertex,rank`:

```
vertex,rank
1,0.25
2,0.27
```

## Датасеты и проверка

```bash
../scripts/fetch_datasets.sh web-Google # скачивание датасета
./gra preprocess --input ../data/snap/web-Google.csv --output wg
./gra run --input wg --output wg/ranks.csv --tol 1e-9
python3 ../scripts/validate.py --edges ../data/snap/web-Google.csv --ranks wg/ranks.csv --tol 1e-9
```

`validate.py` независимо считает LeaderRank на NumPy и сравнивает с нашим выводом
(на web-Google при сходимости до `1e-9` расхождение ~5e-14).

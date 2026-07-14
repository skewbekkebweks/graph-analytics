import argparse

import numpy as np

def load_edges(path):
    src = []
    dst = []
    with open(path) as f:
        first = True
        for line in f:
            line = line.strip()
            if not line:
                continue
            a, b = line.split(",")[:2]
            if first and not a.isdigit():
                first = False
                continue
            first = False
            src.append(int(a))
            dst.append(int(b))
    return np.array(src, dtype=np.int64), np.array(dst, dtype=np.int64)


def leaderrank(src, dst, tol, max_iter):
    nodes = np.unique(np.concatenate([src, dst]))
    n = nodes.size
    s = np.searchsorted(nodes, src)
    d = np.searchsorted(nodes, dst)
    outdeg = np.bincount(s, minlength=n).astype(np.float64)

    cur = np.ones(n, dtype=np.float64)
    sg = 0.0
    for _ in range(max_iter):
        contrib = cur / (outdeg + 1.0)
        sg_next = contrib.sum()
        nxt = np.full(n, sg / n, dtype=np.float64)
        nxt += np.bincount(d, weights=contrib[s], minlength=n)
        diff = np.abs(nxt - cur).sum() + abs(sg_next - sg)
        cur = nxt
        sg = sg_next
        if diff < tol * n:
            break
    rank = cur + sg / n
    rank /= rank.sum()
    return {int(nodes[i]): float(rank[i]) for i in range(n)}


def load_ranks(path):
    ranks = {}
    with open(path) as f:
        next(f)
        for line in f:
            v, r = line.strip().split(",")
            ranks[int(v)] = float(r)
    return ranks


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--edges", required=True)
    ap.add_argument("--ranks", required=True)
    ap.add_argument("--tol", type=float, default=1e-10)
    ap.add_argument("--max-iter", type=int, default=2000)
    args = ap.parse_args()

    src, dst = load_edges(args.edges)
    ref = leaderrank(src, dst, args.tol, args.max_iter)
    ours = load_ranks(args.ranks)

    keys = sorted(ref.keys())
    a = np.array([ref[k] for k in keys])
    b = np.array([ours[k] for k in keys])

    max_abs = np.max(np.abs(a - b))
    mean_abs = np.mean(np.abs(a - b))

    print(f"vertices      : {len(keys)}")
    print(f"max abs diff  : {max_abs:.3e}")
    print(f"mean abs diff : {mean_abs:.3e}")


if __name__ == "__main__":
    main()

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <cmath>
#include <cstddef>
using namespace std;

#define int long long

struct Point {
    long long x, y;
};

long long cross(Point a, Point b) {
    return a.x * b.y - a.y * b.x;
}

Point operator-(Point a, Point b) {
    return {a.x - b.x, a.y - b.y};
}

int sgn(long long x) {
    if (x == 0) return 0;
    return x > 0 ? 1 : -1;
}

bool onSegment(Point a, Point b, Point p) {
    return min(a.x, b.x) <= p.x && p.x <= max(a.x, b.x)
        && min(a.y, b.y) <= p.y && p.y <= max(a.y, b.y);
}

bool intersect(Point a1, Point a2, Point b1, Point b2) {
    Point a = a2 - a1;
    Point b = b2 - b1;
    long long c1 = cross(a, b1 - a1);
    long long c2 = cross(a, b2 - a1);
    long long c3 = cross(b, a1 - b1);
    long long c4 = cross(b, a2 - b1);
    
    if (sgn(c1) * sgn(c2) < 0 && sgn(c3) * sgn(c4) < 0)
        return true;
    
    if (sgn(c1) == 0 && onSegment(a1, a2, b1)) return true;
    if (sgn(c2) == 0 && onSegment(a1, a2, b2)) return true;
    if (sgn(c3) == 0 && onSegment(b1, b2, a1)) return true;
    if (sgn(c4) == 0 && onSegment(b1, b2, a2)) return true;
    
    return false;
}

struct Segment {
    long long id;      // 1-indexed original id
    long long xl, yl;  // left endpoint
    long long xr, yr;  // right endpoint
    bool vertical;
};

vector<Segment> seg;
long long curX;

// numerator of y at x: y(x) * dx, only for non-vertical
__int128 numAt(long long id, long long x) {
    return (__int128)seg[id].yl * (seg[id].xr - seg[id].xl)
         + (__int128)(seg[id].yr - seg[id].yl) * (x - seg[id].xl);
}

long long dx(long long id) {
    return seg[id].xr - seg[id].xl;
}

struct Cmp {
    bool operator()(long long a, long long b) const {
        __int128 left  = numAt(a, curX) * dx(b);
        __int128 right = numAt(b, curX) * dx(a);
        if (left != right) return left < right;
        return a < b;
    }
};

struct Event {
    long long x;
    int type;          // 0 insert, 1 vertical, 2 remove
    long long id;      // index in seg
};

// returns {0,0} if no intersection found
pair<long long,long long> findAnyIntersection(long long n, long long exclude) {
    vector<Event> ev;
    ev.reserve(2 * n);
    for (long long i = 0; i < n; i++) {
        if (seg[i].id == exclude) continue;
        if (seg[i].vertical) {
            ev.push_back({seg[i].xl, 1, i});
        } else {
            ev.push_back({seg[i].xl, 0, i});
            ev.push_back({seg[i].xr, 2, i});
        }
    }
    sort(ev.begin(), ev.end(), [](const Event& a, const Event& b) {
        if (a.x != b.x) return a.x < b.x;
        return a.type < b.type;
    });

    set<long long, Cmp> active;

    for (size_t i = 0; i < ev.size(); ) {
        long long x = ev[i].x;
        curX = x;

        vector<long long> ins, vert, rem;
        size_t j = i;
        while (j < ev.size() && ev[j].x == x) {
            if (ev[j].type == 0) ins.push_back(ev[j].id);
            else if (ev[j].type == 1) vert.push_back(ev[j].id);
            else rem.push_back(ev[j].id);
            ++j;
        }

        // insertions
        for (long long sid : ins) {
            auto it = active.insert(sid).first;
            if (it != active.begin()) {
                long long p = *prev(it);
                if (intersect({seg[p].xl,seg[p].yl}, {seg[p].xr,seg[p].yr},
                              {seg[sid].xl,seg[sid].yl}, {seg[sid].xr,seg[sid].yr}))
                    return {seg[p].id, seg[sid].id};
            }
            auto nit = next(it);
            if (nit != active.end()) {
                long long s = *nit;
                if (intersect({seg[s].xl,seg[s].yl}, {seg[s].xr,seg[s].yr},
                              {seg[sid].xl,seg[sid].yl}, {seg[sid].xr,seg[sid].yr}))
                    return {seg[s].id, seg[sid].id};
            }
        }

        // vertical segments: check active segments whose y at x lies in [yl, yr]
        for (long long sid : vert) {
            long long yl = seg[sid].yl, yr = seg[sid].yr;
            // probe segment at y = yl
            seg[n].xl = x - 1; seg[n].yl = yl;
            seg[n].xr = x + 1; seg[n].yr = yl;
            auto it = active.lower_bound(n);
            while (it != active.end()) {
                long long o = *it;
                __int128 yn = numAt(o, x);
                __int128 yd = dx(o);
                if (yn < (__int128)yl * yd) { ++it; continue; }
                if (yn > (__int128)yr * yd) break;
                if (intersect({seg[sid].xl,seg[sid].yl}, {seg[sid].xr,seg[sid].yr},
                              {seg[o].xl,seg[o].yl}, {seg[o].xr,seg[o].yr}))
                    return {seg[sid].id, seg[o].id};
                ++it;
            }
        }

        // removals
        for (long long sid : rem) {
            auto it = active.find(sid);
            if (it == active.end()) continue;
            auto nit = next(it);
            if (it != active.begin() && nit != active.end()) {
                long long p = *prev(it), s = *nit;
                if (intersect({seg[p].xl,seg[p].yl}, {seg[p].xr,seg[p].yr},
                              {seg[s].xl,seg[s].yl}, {seg[s].xr,seg[s].yr}))
                    return {seg[p].id, seg[s].id};
            }
            active.erase(it);
        }

        i = j;
    }

    return {0, 0};
}

bool hasIntersection(long long n, long long exclude) {
    auto p = findAnyIntersection(n, exclude);
    return p.first != 0;
}

signed main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    long long n;
    cin >> n;
    seg.resize(n + 1); // last one is probe
    for (long long i = 0; i < n; i++) {
        long long x1, y1, x2, y2;
        cin >> x1 >> y1 >> x2 >> y2;
        seg[i].id = i + 1;
        if (x1 == x2) {
            seg[i].vertical = true;
            seg[i].xl = seg[i].xr = x1;
            seg[i].yl = min(y1, y2);
            seg[i].yr = max(y1, y2);
        } else if (x1 < x2) {
            seg[i].vertical = false;
            seg[i].xl = x1; seg[i].yl = y1;
            seg[i].xr = x2; seg[i].yr = y2;
        } else {
            seg[i].vertical = false;
            seg[i].xl = x2; seg[i].yl = y2;
            seg[i].xr = x1; seg[i].yr = y1;
        }
    }

    auto p = findAnyIntersection(n, 0);
    if (p.first == 0) {
        cout << 1 << endl;
        return 0;
    }

    vector<long long> cand = {p.first, p.second};
    sort(cand.begin(), cand.end());
    long long ans = cand[1];
    for (long long c : cand) {
        if (!hasIntersection(n, c)) {
            ans = c;
            break;
        }
    }
    cout << ans << endl;
    return 0;
}

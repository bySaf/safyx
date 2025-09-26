import math
import numpy as np
from objects import Objects

class Eclipse:
    """Ellipse geometry with arc-length parameterization."""

    def __init__(self, x_axis_coef: float, y_axis_coef: float):
        self.x_R = float(x_axis_coef)
        self.y_R = float(y_axis_coef)
        self._total_len_cache = None

    def point_from_param(self, t: float):
        a, b = self.x_R, self.y_R
        return a * math.cos(t), b * math.sin(t)

    def _speed(self, t: float):
        a, b = self.x_R, self.y_R
        return math.sqrt((a * math.sin(t)) ** 2 + (b * math.cos(t)) ** 2)

    def arc_length_between(self, t1: float, t2: float):
        dt = t2 - t1
        if abs(dt) < 1e-15:
            return 0.0
        if dt < 0:
            dt += 2 * math.pi
        N = 300
        h = dt / N
        s = self._speed(t1) + self._speed(t1 + dt)
        for i in range(1, N):
            s += (4 if i % 2 else 2) * self._speed(t1 + i * h)
        return float(s * h / 3.0)

    def total_length(self):
        if self._total_len_cache is None:
            self._total_len_cache = self.arc_length_between(0.0, 2 * math.pi)
        return self._total_len_cache

    def s_to_t(self, s: float, t_start: float = 0.0):
        L = self.total_length()
        s = s % L
        lo, hi = t_start, t_start + 2 * math.pi
        for _ in range(50):
            mid = 0.5 * (lo + hi)
            Smid = self.arc_length_between(t_start, mid)
            if Smid < s:
                lo = mid
            else:
                hi = mid
        return (0.5 * (lo + hi)) % (2 * math.pi)

    def normal_vec(self, t: float):
        x, y = self.point_from_param(t)
        a, b = self.x_R, self.y_R
        nx = x / (a * a) if a != 0 else 0.0
        ny = y / (b * b) if b != 0 else 0.0
        n = math.hypot(nx, ny) or 1.0
        return nx / n, ny / n

    def yaw_user(self, t: float) -> float:
        """Yaw in user convention: 0° = +Y, 90° = +X."""
        nx, ny = self.normal_vec(t)
        return math.atan2(nx, ny)

    def yaw_bullet(self, t: float) -> float:
        """Yaw in PyBullet convention: 0° = +X, 90° = +Y."""
        nx, ny = self.normal_vec(t)
        return math.atan2(ny, nx)


def _gates_list_from_layout(xy_t: np.ndarray, yaw_user: np.ndarray):
    out = []
    for i in range(1, xy_t.shape[0]):
        x, y = float(xy_t[i, 0]), float(xy_t[i, 1])
        yaw_user_deg = (math.degrees(float(yaw_user[i])) + 360.0) % 360.0
        out.append(((x, y), yaw_user_deg))
    return out


class TrackGen:
    """Generate layouts of gates along an ellipse."""

    def __init__(self, ellipse: Eclipse):
        self.ellipse = ellipse

    def _upright_uav_as_gate0_layout(self, n: int):
        """Return (xy_t, yaw_user, ts) for N=n+1 points on ellipse."""
        if n < 0:
            n = 0
        N_total = n + 1
        if N_total == 0:
            return np.zeros((0, 2)), np.zeros((0,)), []

        L = self.ellipse.total_length()
        spacing = L / N_total
        t0 = math.pi / 2.0  # tangent along −X
        ts = [self.ellipse.s_to_t(k * spacing, t_start=t0) for k in range(N_total)]

        xy = np.array([self.ellipse.point_from_param(t) for t in ts], dtype=float)
        offset = -xy[0].copy()
        xy_t = xy + offset

        yaw_user = np.array([self.ellipse.yaw_user(t) for t in ts], dtype=float)
        return xy_t, yaw_user, ts

    def spawn_gates_in_sim_with_heights(self, pyb_client_id, n: int,
                                        z_base: float = 1.0,
                                        delta_z_list=None):
        xy_t, yaw_user, ts = self._upright_uav_as_gate0_layout(n)
        gates_raw = _gates_list_from_layout(xy_t, yaw_user)

        if delta_z_list is None:
            delta_z_list = [0.0] * n
        if len(delta_z_list) < n:
            delta_z_list = list(delta_z_list) + [0.0] * (n - len(delta_z_list))

        delta_x, delta_y = 0.85, 2.5
        ids = []
        for i in range(1, len(ts)):
            x, y = float(xy_t[i, 0]), float(xy_t[i, 1])
            z_i = float(z_base + delta_z_list[i - 1])
            yaw_bull = self.ellipse.yaw_bullet(ts[i])
            gate = Objects(
                pyb_client_id, "base_gate",
                coordinates=[x - delta_x, y - delta_y, z_i],
                angles=[0.0, 0.0, yaw_bull]
            )
            ids.append(gate.spawn())

        drone_start = (0.0, 0.0, z_base)
        offset = (0.0, 0.0)
        return gates_raw, ids, offset, drone_start

    def spawn_gates_in_sim(self, pyb_client_id, n: int, z: float = 1.0):
        return self.spawn_gates_in_sim_with_heights(
            pyb_client_id, n, z_base=z, delta_z_list=[0.0] * n
        )


class TrackRandomizer:
    """Seed-based generator of randomized tracks."""

    def __init__(self,
                 a_range=(5.0, 6.0),
                 b_range=(3.0, 4.0),
                 z_base=0.0,
                 dz_range=(-0.5, 0.0)):
        self.a_range = (float(min(a_range)), float(max(a_range)))
        self.b_range = (float(min(b_range)), float(max(b_range)))
        self.z_base = float(z_base)
        self.dz_range = (float(min(dz_range)), float(max(dz_range)))

    def _rng(self, seed: int):
        return np.random.default_rng(int(seed))

    def sample_params(self, seed: int, n: int):
        rng = self._rng(seed)
        a = rng.uniform(*self.a_range)
        b = rng.uniform(*self.b_range)
        dz = rng.uniform(self.dz_range[0], self.dz_range[1], size=n).astype(float)
        return a, b, dz

    def generate_and_spawn(self, pyb_client_id, seed: int, n: int):
        a, b, dz = self.sample_params(seed, n)
        print(f"[TrackRandomizer] SEED={seed} a={a:.3f} b={b:.3f} "
              f"dz∈[{dz.min():.3f},{dz.max():.3f}]")

        ellipse = Eclipse(a, b)
        track = TrackGen(ellipse)
        result = track.spawn_gates_in_sim_with_heights(
            pyb_client_id, n, z_base=self.z_base, delta_z_list=dz.tolist()
        )
        params = {"seed": int(seed), "a": float(a), "b": float(b),
                  "z_base": float(self.z_base), "delta_z": dz.tolist()}
        return seed, params, result

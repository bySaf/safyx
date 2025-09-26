import random
from dataclasses import dataclass
from gym_pybullet_drones.utils.enums import DroneModel, Physics


# Centralized configuration for the simulator and the first‑person camera.

@dataclass
class SimConfig:
    drone_model: DroneModel = DroneModel("cf2x")
    num_drones: int = 1
    physics: Physics = Physics("pyb")
    use_gui: bool = True  # must be True for hardware renderer
    record_video: bool = False
    plot_results: bool = True
    user_debug_gui: bool = False
    obstacles: bool = False
    set_seed: int = 312313
    num_gates: int = 7
    # set_seed: int = random.randint(1, 25042008)
    # put -1 for random seed generation

    sim_freq_hz: int = 240
    ctrl_freq_hz: int = 48
    duration_sec: int = 100
    output_folder: str = "results"

    # Optional: spawn gates/track for visuals
    spawn_track: bool = True


@dataclass
class CameraConfig:
    enabled: bool = True
    width: int = 480
    height: int = 270
    fov_deg: float = 70
    near: float = 0.002
    far: float = 12.0
    window_name: str = "Drone cam (mounted)"
    frame_stride: int = 2  # render every Nth control step

    # Mount in body frame: x=fwd, y=right, z=up (meters)
    mount_forward_m: float = 0.07
    mount_right_m: float = 0.00
    mount_up_m: float = 0.02
    look_ahead_m: float = 2.0

    # IMPORTANT: CF2x forward is −X. If your model points +X forward, set +1.
    body_forward_sign: int = -1  # -1: −X is nose; +1: +X is nose
    body_right_sign: int = 1

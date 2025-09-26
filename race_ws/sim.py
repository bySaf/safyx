from __future__ import annotations
import time
import math
import numpy as np
import pybullet as p
import random
from typing import Callable, Optional

from gym_pybullet_drones.utils.enums import DroneModel
from gym_pybullet_drones.envs.CtrlAviary import CtrlAviary
from gym_pybullet_drones.envs.BetaAviary import BetaAviary
from gym_pybullet_drones.control.CTBRControl import CTBRControl

from gym_pybullet_drones.control.DSLPIDControl import DSLPIDControl
from gym_pybullet_drones.utils.Logger import Logger
from gym_pybullet_drones.utils.utils import sync

from config import SimConfig, CameraConfig
from camera import DroneCamera
from generator import TrackRandomizer


class DroneSim:
    """Simulation wrapper with optional task hook."""

    def __init__(self, sim_cfg: SimConfig, cam_cfg: CameraConfig):
        self.cfg = sim_cfg
        self.cam_cfg = cam_cfg

        self.env = None
        self.controllers = None
        self.logger = None
        self.camera = None
        self.client_id = None

        # Initial ring around (0, -r)
        r = 0.3
        base_z = 0
        z_step = 0.05
        self.init_xyzs = np.array([
            [
                r * np.cos((i / self.cfg.num_drones) * 2 * np.pi + np.pi / 2),
                r * np.sin((i / self.cfg.num_drones) * 2 * np.pi + np.pi / 2) - r,
                base_z + i * z_step
            ]
            for i in range(self.cfg.num_drones)
        ])
        self.init_rpys = np.array([[0, 0, i * (np.pi / 2) / self.cfg.num_drones] for i in range(self.cfg.num_drones)])

    def build(self):

        MODEL_MAP = {
            DroneModel.CF2X: (DSLPIDControl, CtrlAviary),
            DroneModel.CF2P: (DSLPIDControl, CtrlAviary),
            DroneModel.RACE: (CTBRControl, BetaAviary),
        }

        ctrl_cls, env_cls = MODEL_MAP.get(self.cfg.drone_model, (None, None))
        if ctrl_cls is None or env_cls is None:
            raise ValueError(f"Unsupported drone_model={self.cfg.drone_model}")

        self.controllers = [ctrl_cls(drone_model=self.cfg.drone_model)
                            for _ in range(self.cfg.num_drones)]

        self.env = env_cls(
            drone_model=self.cfg.drone_model,
            num_drones=self.cfg.num_drones,
            initial_xyzs=self.init_xyzs,
            initial_rpys=self.init_rpys,
            physics=self.cfg.physics,
            neighbourhood_radius=10,
            pyb_freq=self.cfg.sim_freq_hz,
            ctrl_freq=self.cfg.ctrl_freq_hz,
            gui=self.cfg.use_gui,
            record=self.cfg.record_video,
            obstacles=self.cfg.obstacles,
            user_debug_gui=self.cfg.user_debug_gui,
        )

        self.logger = Logger(
            logging_freq_hz=self.cfg.ctrl_freq_hz,
            num_drones=self.cfg.num_drones,
            output_folder=self.cfg.output_folder,
            colab=False,

        )

        self.client_id = self.env.getPyBulletClient()

        if self.cfg.spawn_track:
            TrackRandomizer().generate_and_spawn(pyb_client_id=self.client_id,
                                                 seed=self.cfg.set_seed, n=self.cfg.num_gates)

        if self.cam_cfg.enabled:
            self.camera = DroneCamera(client_id=self.client_id, cfg=self.cam_cfg)

    def run(self, task_fn: Optional[Callable] = None):
        """Run the sim. If task_fn is None, drones hover (zero actions)."""
        self.build()
        actions = np.zeros((self.cfg.num_drones, 4))
        start = time.time()

        steps = int(self.cfg.duration_sec * self.env.CTRL_FREQ)
        for i in range(steps):
            obs, _, _, _, _ = self.env.step(actions)

            if task_fn is not None:
                task_fn(self, i, obs, actions)

            # Logging expects 12-dim "control" vector, not just 4-dim motor cmd
            for j in range(self.cfg.num_drones):
                control_vec = np.hstack([
                    np.zeros(3),  # desired position (unused here)
                    np.zeros(3),  # desired rpy (unused here)
                    np.zeros(6)  # fill to length 12
                ])
                self.logger.log(
                    drone=j,
                    timestamp=i / self.env.CTRL_FREQ,
                    state=obs[j],
                    control=control_vec,
                )

            self.env.render()
            if self.camera is not None:
                try:
                    pos, yaw_deg, quat = self.get_pose(0)
                    if not self.camera.render(pos, yaw_deg, body_quat=quat):
                        break
                except Exception:
                    pass

            if self.cfg.use_gui:
                sync(i, start, self.env.CTRL_TIMESTEP)

        self.close()

    def get_pose(self, drone_index: int = 0):
        drone_id = self.env.DRONE_IDS[drone_index]
        pos, orn = p.getBasePositionAndOrientation(drone_id, physicsClientId=self.client_id)
        rpy = p.getEulerFromQuaternion(orn)
        yaw_deg = math.degrees(rpy[2])
        return np.array(pos), yaw_deg, np.array(orn)

    def close(self):
        if self.env is not None:
            self.env.close()
        if self.camera is not None:
            self.camera.close()
        if self.logger is not None:
            self.logger.save()
            self.logger.save_as_csv(self.cfg.set_seed)
            if self.cfg.plot_results:
                self.logger.plot()

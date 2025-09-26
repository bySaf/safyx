"""First‑person camera that follows the drone's full orientation (R/P/Y).
The drone model is never in view (the camera is mounted in the nose).
"""
from __future__ import annotations
import math
import numpy as np
import cv2
import pybullet as p
from typing import Sequence

from config import CameraConfig


def _view_from_eye_target(eye, target, up):
    return p.computeViewMatrix(cameraEyePosition=eye,
                               cameraTargetPosition=target,
                               cameraUpVector=up)


class DroneCamera:
    def __init__(self, client_id: int, cfg: CameraConfig):
        self.client = client_id
        self.cfg = cfg
        self.W = int(cfg.width)
        self.H = int(cfg.height)
        self._frame_id = 0

        # Pick renderer (hardware GL if GUI, else tiny)
        try:
            info = p.getConnectionInfo()
            self.renderer = (
                p.ER_BULLET_HARDWARE_OPENGL if info.get("connectionMethod", -1) == p.GUI else p.ER_TINY_RENDERER
            )
        except Exception:
            self.renderer = p.ER_TINY_RENDERER

        aspect = self.W / self.H
        self.proj_mat = p.computeProjectionMatrixFOV(cfg.fov_deg, aspect, cfg.near, cfg.far)

        try:
            cv2.namedWindow(cfg.window_name, cv2.WINDOW_NORMAL)
            cv2.resizeWindow(cfg.window_name, self.W, self.H)
        except Exception:
            pass

    # ---------- math utils ----------
    @staticmethod
    def _quat_to_axes(q: Sequence[float]):
        """quat(x,y,z,w) -> body axes in world: x_fwd, y_right, z_up"""
        x, y, z, w = q
        xx, yy, zz = x*x, y*y, z*z
        xy, xz, yz = x*y, x*z, y*z
        wx, wy, wz = w*x, w*y, w*z
        R = np.array([
            [1-2*(yy+zz), 2*(xy-wz),   2*(xz+wy)],
            [2*(xy+wz),   1-2*(xx+zz), 2*(yz-wx)],
            [2*(xz-wy),   2*(yz+wx),   1-2*(xx+yy)]
        ], dtype=np.float32)
        x_fwd = R @ np.array([1, 0, 0], dtype=np.float32)
        y_right = R @ np.array([0, 1, 0], dtype=np.float32)
        z_up = R @ np.array([0, 0, 1], dtype=np.float32)
        return x_fwd, y_right, z_up

    @staticmethod
    def _rgba_to_bgr(rgba_like, w: int, h: int):
        """RGBA (ndarray/bytes) -> BGR (h,w,3)."""
        if isinstance(rgba_like, np.ndarray):
            arr = rgba_like
            if arr.ndim == 3 and arr.shape[-1] == 4:
                pass
            else:
                arr = np.array(arr, dtype=np.uint8).reshape(h, w, 4)
        else:
            arr = np.frombuffer(rgba_like if isinstance(rgba_like, (bytes, bytearray)) else bytes(rgba_like), dtype=np.uint8)
            arr = arr.reshape(h, w, 4)
        return cv2.cvtColor(arr[:, :, :3], cv2.COLOR_RGB2BGR)

    def render(self, body_pos, yaw_deg_legacy, body_quat=None) -> bool:
        """Render mounted first‑person view.
        Returns False if the user pressed 'q' in the OpenCV window, True otherwise.
        """
        self._frame_id += 1
        if (self._frame_id % self.cfg.frame_stride) != 0:
            return True

        if body_quat is not None:
            x_fwd, y_right, z_up = self._quat_to_axes(body_quat)
            x_fwd = self.cfg.body_forward_sign * x_fwd
            y_right = self.cfg.body_right_sign * y_right

            eye = (np.array(body_pos, dtype=np.float32)
                   + x_fwd * self.cfg.mount_forward_m
                   + y_right * self.cfg.mount_right_m
                   + z_up * self.cfg.mount_up_m)

            target = eye + x_fwd * self.cfg.look_ahead_m
            up = z_up
        else:
            # Legacy yaw-only rendering
            yaw = math.radians(yaw_deg_legacy)
            forward = np.array([math.cos(yaw), math.sin(yaw), 0.0], dtype=np.float32)
            up = np.array([0.0, 0.0, 1.0], dtype=np.float32)
            eye = np.array(body_pos, dtype=np.float32) - forward * 0.1 + up * 0.03
            target = np.array(body_pos, dtype=np.float32)

        view = _view_from_eye_target(eye.tolist(), target.tolist(), up.tolist())
        img = p.getCameraImage(self.W, self.H, view, self.proj_mat, renderer=self.renderer)

        bgr = self._rgba_to_bgr(img[2], self.W, self.H)

        cv2.imshow(self.cfg.window_name, bgr)
        return not (cv2.waitKey(1) & 0xFF == ord('q'))

    def close(self):
        try:
            cv2.destroyWindow(self.cfg.window_name)
        except Exception:
            pass
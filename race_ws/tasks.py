import numpy as np


def circle_demo_task(sim, step_i: int, obs, actions) -> None:
    """XY circle at fixed Z using DSLPID controllers attached in sim."""
    if not hasattr(sim, "_circle_wp"):
        ctrl_freq = sim.cfg.ctrl_freq_hz
        period = 10  # seconds per revolution
        r = 0.3
        num_wp = ctrl_freq * period
        sim._circle_wp = np.zeros((num_wp, 3), dtype=float)
        cx, cy = sim.init_xyzs[0, 0], sim.init_xyzs[0, 1] - r
        for i in range(num_wp):
            theta = (i / num_wp) * (2 * np.pi) + np.pi / 2
            x = r * np.cos(theta) + cx
            y = r * np.sin(theta) + cy
            sim._circle_wp[i, :] = [x, y, 0.0]
        sim._circle_num = num_wp
        sim._circle_ptr = np.array([int((i * num_wp / max(1, sim.cfg.num_drones)) % num_wp)
                                    for i in range(sim.cfg.num_drones)])

    for j in range(sim.cfg.num_drones):
        target_xyz = np.hstack([sim._circle_wp[sim._circle_ptr[j], 0:2], sim.init_xyzs[j, 2]])
        actions[j, :], _, _ = sim.controllers[j].computeControlFromState(
            control_timestep=sim.env.CTRL_TIMESTEP,
            state=obs[j],
            target_pos=target_xyz,
            target_rpy=sim.init_rpys[j, :]
        )
        sim._circle_ptr[j] = (sim._circle_ptr[j] + 1) % sim._circle_num

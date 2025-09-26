import argparse

from config import SimConfig, CameraConfig
from sim import DroneSim
from tasks import circle_demo_task



def parse_args():
    p = argparse.ArgumentParser()
    p.add_argument("--circle", action="store_true", help="Run the built-in demo circle task")
    return p.parse_args()


def main():
    args = parse_args()

    sim_cfg = SimConfig()
    cam_cfg = CameraConfig()

    sim = DroneSim(sim_cfg, cam_cfg)

    if args.circle:
        sim.run(task_fn=circle_demo_task)
    else:
        sim.run(task_fn=None)


if __name__ == "__main__":
    main()
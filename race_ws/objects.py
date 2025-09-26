import pybullet as p
from pathlib import Path


class Objects:
    """Wrapper for spawning static URDF objects (like gates) into PyBullet."""

    _base = Path(__file__).resolve().parent

    _registry = {
        "base_gate": "track_assets/race_gate.urdf",
        # TODO: Other types of objects
        # "vert_gate": "track_assets/vert_gates.urdf",
        # "start_end": "track_assets/h_map.urdf",
    }

    def __init__(self, pyb_client_id, obj_type, coordinates=None, angles=None):
        if obj_type not in self._registry:
            raise ValueError(
                f"Unknown object type '{obj_type}'. "
                f"Available: {list(self._registry.keys())}"
            )

        if coordinates is None:
            coordinates = [0.0, 0.0, 0.0]
        self._validate_vector(coordinates, name="coordinates", limit=100)

        if angles is None:
            angles = [0.0, 0.0, 0.0]
        self._validate_vector(angles, name="angles", limit=180)

        self.client_id = pyb_client_id
        self.coordinates = coordinates
        self.angles = p.getQuaternionFromEuler(angles)
        self.path = str(self._base / self._registry[obj_type])

    def spawn(self):
        """Spawn the URDF object and return its body ID."""
        print(self.path)
        return p.loadURDF(
            fileName=self.path,
            basePosition=self.coordinates,
            baseOrientation=self.angles,
            physicsClientId=self.client_id
        )

    @staticmethod
    def _validate_vector(vec, name, limit=None):
        if not isinstance(vec, (list, tuple)):
            raise TypeError(f"{name} must be list or tuple, got {type(vec)}")
        if len(vec) != 3:
            raise ValueError(f"{name} must have 3 elements, got {len(vec)}")
        if not all(isinstance(x, (int, float)) for x in vec):
            raise TypeError(f"All elements of {name} must be numbers")
        if limit is not None:
            for obj in vec:
                if obj > limit or obj < -limit:
                    raise ValueError(f"{name} element {obj} exceeds limit ±{limit}")


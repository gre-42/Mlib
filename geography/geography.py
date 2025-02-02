import numpy as np


kilo = 1e3
meters = 1
degrees = np.pi / 180
radians = 1


def latitude_longitude_2_meters(
        latitude: float,
        longitude: float,
        latitude0: float,
        longitude0: float) -> np.ndarray:
    r0 = 6_371 * kilo * meters
    r1 = r0 * np.cos(latitude0 * degrees / radians)
    circumference0 = r0 * 2 * np.pi
    circumference1 = r1 * 2 * np.pi
    return np.array((
        (circumference1 / 360) * (longitude - longitude0),
        (circumference0 / 360) * (latitude - latitude0)),
        dtype=float)


class TransformationMatrix:
    t: np.ndarray
    R: np.ndarray

    def transformed(self, a: np.ndarray):
        return self.t + np.dot(self.R, a.T).T
    
    @property
    def inverse(self) -> 'TransformationMatrix':
        inv = np.linalg.inv(self.affine)
        res = TransformationMatrix()
        res.t = inv[:-1, -1]
        res.R = inv[:-1, :-1]
        return res

    @property
    def affine(self) -> np.ndarray:
        res = np.identity(len(self.t) + 1)
        res[:-1, -1] = self.t
        res[:-1, :-1] = self.R
        return res

    def __str__(self) -> str:
        return f'TransformationMatrix({self.t}, {self.R})'


def latitude_longitude_2_meters_mapping(
        latitude0: float,
        longitude0: float) -> TransformationMatrix:
    '''
    Compute a transformation matrix that maps geographic coordinates to meters.

    Wrapper around latitude_longitude_2_meters (multiply by zeros and the identity matrix)
    to get a transformation matrix.
    '''
    result = TransformationMatrix()
    result.t = latitude_longitude_2_meters(0, 0, latitude0, longitude0)
    result.R = np.empty(shape=(2, 2))
    result.R[:, 0] = latitude_longitude_2_meters(
        1, 0, latitude0, longitude0) - result.t
    result.R[:, 1] = latitude_longitude_2_meters(
        0, 1, latitude0, longitude0) - result.t
    return result

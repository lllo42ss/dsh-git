import unittest

from simulation.ball_beam import BallBeam, simulate_hold


class BallBeamTests(unittest.TestCase):
    def test_positive_angle_moves_ball_positive(self):
        plant = BallBeam()
        for _ in range(50):
            plant.step(5.0, 0.01)
        self.assertGreater(plant.x_m, 0.0)

    def test_hold_from_positive_offset(self):
        result = simulate_hold(target_cm=0.0, start_cm=5.0, seconds=6.0)
        self.assertLess(result.max_error_cm, 6.0)
        self.assertLess(result.final_error_cm, 1.0)
        self.assertTrue(result.settled)


if __name__ == "__main__":
    unittest.main()

import unittest

from simulation.control import Pid, clamp, mix_motors, weighted_line_error
from simulation.line_follow import steering_command


class ControlTests(unittest.TestCase):
    def test_clamp(self):
        self.assertEqual(clamp(5, 0, 3), 3)
        self.assertEqual(clamp(-1, 0, 3), 0)

    def test_mix_motors_limits(self):
        left, right = mix_motors(0.9, 0.5)
        self.assertLessEqual(left, 1.0)
        self.assertGreaterEqual(right, -1.0)

    def test_pid_reaches_setpoint_direction(self):
        pid = Pid(2.0, 0.1, 0.0, -10.0, 10.0)
        output = pid.update(3.0, 0.0, 0.01)
        self.assertGreater(output, 0)

    def test_line_error_center(self):
        error, lost = weighted_line_error([0.0, 0.0, 1.0, 0.0, 0.0], [-2, -1, 0, 1, 2])
        self.assertFalse(lost)
        self.assertAlmostEqual(error, 0.0)

    def test_line_lost(self):
        error, lost = weighted_line_error([0.0, 0.0, 0.0, 0.0, 0.0], [-2, -1, 0, 1, 2])
        self.assertTrue(lost)
        self.assertEqual(error, 0.0)

    def test_steering_direction(self):
        left, right, lost = steering_command([0.0, 0.0, 0.0, 1.0, 0.0])
        self.assertFalse(lost)
        self.assertNotEqual(left, right)


if __name__ == "__main__":
    unittest.main()

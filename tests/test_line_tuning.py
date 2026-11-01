import unittest

from simulation.line_follow import steering_command


class LineTuningTests(unittest.TestCase):
    def test_half_circle_bias_is_bounded(self):
        left, right, lost = steering_command([0, 0.2, 0.8, 0.2, 0])
        self.assertFalse(lost)
        self.assertLessEqual(abs(left - right), 1.0)

    def test_lost_returns_zero(self):
        self.assertEqual(steering_command([0, 0, 0, 0, 0]), (0.0, 0.0, True))


if __name__ == "__main__":
    unittest.main()

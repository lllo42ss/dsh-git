import unittest

from simulation.line_follow import WEIGHTS, steering_command


class LineFollowTests(unittest.TestCase):
    def test_center_goes_straight(self):
        left, right, lost = steering_command([0, 0, 1, 0, 0])
        self.assertFalse(lost)
        self.assertAlmostEqual(left, right)

    def test_right_sensor_turns_right(self):
        left, right, lost = steering_command([0, 0, 0, 0, 1])
        self.assertFalse(lost)
        self.assertGreater(left, right)

    def test_all_white_is_lost(self):
        left, right, lost = steering_command([0, 0, 0, 0, 0])
        self.assertTrue(lost)
        self.assertEqual((left, right), (0.0, 0.0))

    def test_weights_are_symmetric(self):
        self.assertEqual(WEIGHTS, [-2.0, -1.0, 0.0, 1.0, 2.0])


if __name__ == "__main__":
    unittest.main()

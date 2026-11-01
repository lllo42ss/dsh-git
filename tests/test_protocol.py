import unittest

from vision.protocol import BallObservation, encode_observation, parse_observation


class ProtocolTests(unittest.TestCase):
    def test_round_trip(self):
        observation = BallObservation(-1.25, 0.9, 12345)
        line = encode_observation(observation)
        parsed = parse_observation(line)
        self.assertIsNotNone(parsed)
        assert parsed is not None
        self.assertAlmostEqual(parsed.x_cm, -1.25)
        self.assertAlmostEqual(parsed.confidence, 0.9)
        self.assertEqual(parsed.stamp_ms, 12345)
        self.assertTrue(parsed.valid)

    def test_low_confidence_invalid(self):
        parsed = parse_observation("B,0.0,0.1,1")
        self.assertIsNotNone(parsed)
        assert parsed is not None
        self.assertFalse(parsed.valid)

    def test_bad_line(self):
        self.assertIsNone(parse_observation("X,1,2,3"))
        self.assertIsNone(parse_observation("B,nope,2,3"))


if __name__ == "__main__":
    unittest.main()

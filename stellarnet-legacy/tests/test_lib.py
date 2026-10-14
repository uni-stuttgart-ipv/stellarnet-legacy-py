import unittest
import src.stellarnet_legacy as sn


class TestLib(unittest.TestCase):
    def stopTestRun(self):
        sn.close()
        sn.kill()

    def test_stellarnet_path(self):
        sn.init()
        sn.open()
        path = sn.stellarnet_path()
        if path is None:
            raise RuntimeError("stellarnet path could not be obtained")

        self.assertTrue("StellarNet\\SpectraWiz" in path)


if __name__ == "__main__":
    unittest.main()

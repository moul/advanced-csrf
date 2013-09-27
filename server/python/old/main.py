from acsrf import Acsrf
from sys import argv

def start():
    port = argv[1] if len(argv) > 1 else 8000
    acsrf = Acsrf(port = port)
    acsrf.run()

if __name__ == "__main__":
    start()

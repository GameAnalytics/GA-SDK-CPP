import base64
import hashlib
import hmac
import sys


def hmac_hash_with_secret(key, message):
    return base64.b64encode(hmac.new(key, message, digestmod=hashlib.sha256).digest())


def run():
    print hmac_hash_with_secret("test1", "test2")

    sys.exit()


run()

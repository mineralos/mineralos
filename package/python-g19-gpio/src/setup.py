from setuptools import setup, find_packages

args = dict(
    name='g19-gpio',
    description=('g19 gpio library'),
    packages=['g19'],
    install_requires=["cffi>=1.0.0"],
    setup_requires=["cffi>=1.0.0"],
    cffi_modules=["./g19/build_gpio.py:ffi"])

setup(**args)

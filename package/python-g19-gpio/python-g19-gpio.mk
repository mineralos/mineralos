################################################################################
#
# python-g19-gpio
#
################################################################################
PYTHON_G19_GPIO_VERSION = local
PYTHON_G19_GPIO_SITE = $(PYTHON_G19_GPIO_PKGDIR)/src
PYTHON_G19_GPIO_SITE_METHOD = local
PYTHON_G19_GPIO_SETUP_TYPE = setuptools
PYTHON_G19_GPIO_DEPENDENCIES = host-python-cffi

$(eval $(python-package))

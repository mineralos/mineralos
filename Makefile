# This file is part of buildroot-submodule.
#
#    buildroot-submodule is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    buildroot-submodule is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with buildroot-submodule.  If not, see <http://www.gnu.org/licenses/>. 2

#Makefile for the anonymous (default) project
PROJECT_NAME := dragonmint_t1 dragonmint_b29 dragonmint_b52 inno_t2 inno_a8plus inno_d9 inno_s11 inno_a5 inno_a6 inno_t1

default:
	git submodule update --init --recursive
	$(foreach project, $(PROJECT_NAME), $(MAKE) PROJECT_NAME=$(project) --file=common.mk;)

.PHONY: t1 t1.% t2 t2.% b29 b29.% b52 b52.% a8plus a8plus.% d9 d9.% s11 s11.% a5 a5.% a6 a6.% t1i t1i.% % s9 s9.%

t1:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=dragonmint_t1 --file=common.mk

t1.%:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=dragonmint_t1 --file=common.mk $*

t2:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=inno_t2 --file=common.mk

t2.%:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=inno_t2 --file=common.mk $*

b29:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=dragonmint_b29 --file=common.mk

b29.%:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=dragonmint_b29 --file=common.mk $*

b52:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=dragonmint_b52 --file=common.mk

b52.%:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=dragonmint_b52 --file=common.mk $*

a8plus:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=inno_a8plus --file=common.mk

a8plus.%:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=inno_a8plus --file=common.mk $*

d9:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=inno_d9 --file=common.mk

d9.%:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=inno_d9 --file=common.mk $*

s11:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=inno_s11 --file=common.mk

s11.%:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=inno_s11 --file=common.mk $*

a5:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=inno_a5 --file=common.mk

a5.%:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=inno_a5 --file=common.mk $*

a6:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=inno_a6 --file=common.mk

a6.%:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=inno_a6 --file=common.mk $*

t1i:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=inno_t1 --file=common.mk

t1i.%:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=inno_t1 --file=common.mk $*

s9:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=antminer_s9 --file=common.mk

s9.%:
	git submodule update --init --recursive
	$(MAKE) PROJECT_NAME=antminer_s9 --file=common.mk $*

%:
	git submodule update --init --recursive
	$(foreach project, $(PROJECT_NAME), $(MAKE) PROJECT_NAME=$(project) --file=common.mk $@;)

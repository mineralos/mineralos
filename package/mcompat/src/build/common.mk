MAP=$(OBJ_DIR)/$(TARGET).map
LST=$(OBJ_DIR)/$(TARGET).lst

all: $(TARGET)
	$(MAKE) -i $(LINT_F)

$(TARGET): $(OBJ)
	@echo [Link...... $@]
	$(QUIET)$(LD) $(LDFLAGS) -o $@ $^
	@echo [Gen....... $@.map]
	$(QUIET)$(READELF) $@ > $(MAP)
	@echo [Gen....... $@.lst]
	$(QUIET)$(OBJDUMP) $@ > $(LST)
	@echo [Strip..... $@]
	$(QUIET)$(STRIP) $(TARGET)

$(OBJ_DIR)/%.o : %.c
	@echo [Compile... $(@F)]
	$(QUIET)$(CC) $(INCLUDE) $(CFLAGS) $(OBJ_DIR)/$(@F) $<

$(OBJ_DIR)/%.lint : %.c
	@echo [Lint...... $(@F)]
	$(QUIET)$(LINT) $(LINT_FLAGS) $(INCLUDE) $< > $(OBJ_DIR)/$(@F) 2>&1

clean:
	$(RM) $(OBJ_DIR)/* $(TARGET)

print_all: print_cfg print_dir print_ct print_out

print_cfg:
	@echo
	@echo "------------------------------ Print Cfg ---------------------------------"
	@echo "VPATH:         $(VPATH)"
	@echo "INCLUDE:       $(INCLUDE)"
	@echo "C_FILES:       $(C_FILES)"
	@echo "------------------------------ Print Cfg End -----------------------------"
	@echo
print_dir:
	@echo
	@echo "------------------------------ Print Dir ---------------------------------"
	@echo "ROOT_DIR:      $(ROOT_DIR)"
	@echo "SRC_DIR:       $(SRC_DIR)"
	@echo "RELEASE_DIR:   $(RELEASE_DIR)"
	@echo "BUILD_DIR:     $(BUILD_DIR)"
	@echo "OBJ_DIR:       $(OBJ_DIR)"
	@echo "------------------------------ Print Dir End -----------------------------"
	@echo
print_ct:
	@echo
	@echo "------------------------------ Print Ct End ------------------------------"
	@echo "CC:            $(CC)"
	@echo "LD:            $(LD)"
	@echo "AR:            $(AR)"
	@echo "STRIP:         $(STRIP)"
	@echo "CP:            $(CP)"
	@echo "CFLAGS:        $(CFLAGS)"
	@echo "LDFLAGS:       $(LDFLAGS)"
	@echo "QUIET:         $(QUIET)"
	@echo "SILENCE:       $(SILENCE)"
	@echo "PLATFORM:      $(PLATFORM)"
	@echo "------------------------------ Print Ct End ------------------------------"
	@echo

print_out:
	@echo
	@echo "------------------------------ Print Out End -----------------------------"
	@echo "TARGET:        $(TARGET)"
	@echo "OBJ:           $(OBJ)"
	@echo "------------------------------ Print Out End -----------------------------"
	@echo

.PHONY: clean print_all print_cfg print_dir print_ct print_out


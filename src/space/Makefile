
TARGETS=auxil libmin getepslay green schur highlay makegln makemesh makedela \
	makefem makesize makesubres mplot tabs tecc space space2 space3d Xspace

install:
	@for dir in $(TARGETS); do \
		echo "### Building" $$dir; make -C $$dir; \
	done
clean:
	@for dir in $(TARGETS); do \
		make -C $$dir clean; \
	done

#EOF

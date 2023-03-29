CPP = cl
CPPFLAGS = /Wall /WX /EHs

SVCNAME = svc.exe
WINKEYNAME = winkey.exe

SVCOBJS = svc.obj
WINKEYOBJS = winkey.obj

all: $(SVCNAME)

$(SVCNAME):
-out:$(SVCNAME) $(SVCOBJS)

$(WINKEYNAME):
-out:$(WINKEYNAME) $(WINKEYOBJS)

clean:
 del $(SVCOBJS) $(WINKEYOBJS)

fclean: clean
 del $(SVCNAME) $(WINKEYNAME)

re: fclean all
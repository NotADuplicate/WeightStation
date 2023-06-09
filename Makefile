CC = gcc
GTK_LIBS = `pkg-config --cflags --libs gtk+-3.0`
LIBS = -ljansson -lcurl -lm
THREADS = -lpthread
OBJS = UI/Window.o Curl/roster.o Helpers/playerHelper.o Helpers/jsonReader.o UI/login.o Curl/cams.o UI/dmx.o HeadshotImporter/headshotRoster.o HeadshotImporter/headshot.o HeadshotImporter/headshotUI.o Curl/schedule.o Curl/survey.o UI/surveyUI.o ServerClient/client.o ServerClient/server.o UI/error.o UI/weighUI.o

program: $(OBJS)
	$(CC) -o program $(OBJS) $(GTK_LIBS) $(LIBS)
	
HeadshotImporter/headshot.o: HeadshotImporter/headshot.c HeadshotImporter/headshot.h
	$(CC) -c HeadshotImporter/headshot.c -o HeadshotImporter/headshot.o $(GTK_LIBS)
	
HeadshotImporter/headshotUI.o: HeadshotImporter/headshotUI.c HeadshotImporter/headshotUI.h
	$(CC) -c HeadshotImporter/headshotUI.c -o HeadshotImporter/headshotUI.o $(GTK_LIBS)
	
HeadshotImporter/headshotRoster.o: HeadshotImporter/headshotRoster.c HeadshotImporter/headshotRoster.h HeadshotImporter/headshot.h 
	$(CC) -c HeadshotImporter/headshotRoster.c -o HeadshotImporter/headshotRoster.o $(LIBS)
	
ClientServer/client.o: ClientServer/client.c ClientServer/client.h
	$(CC) -c ClientServer/client.c -o ClientServer/client.o $(THREADS)

ClientServer/server.o: ClientServer/server.c ClientServer/server.h
	$(CC) -c ClientServer/server.c -o ClientServer/server.o $(THREADS)

UI/Window.o: UI/Window.c Curl/roster.h UI/dmx.h
	$(CC) -c UI/Window.c -o UI/Window.o $(GTK_LIBS)

Curl/roster.o: Curl/roster.c Curl/roster.h
	$(CC) -c Curl/roster.c -o Curl/roster.o $(LIBS)
	
Helpers/playerHelper.o: Helpers/playerHelper.c Helpers/playerHelper.h
	$(CC) -c Helpers/playerHelper.c -o Helpers/playerHelper.o $(LIBS)
	
Helpers/jsonReader.o: Helpers/jsonReader.c Helpers/jsonReader.h
	$(CC) -c Helpers/jsonReader.c -o Helpers/jsonReader.o $(LIBS)
	
Curl/cams.o: Curl/cams.c Curl/cams.h
	$(CC) -c Curl/cams.c -o Curl/cams.o $(LIBS)
	
Curl/survey.o: Curl/survey.c Curl/survey.h
	$(CC) -c Curl/survey.c -o Curl/survey.o $(LIBS)
	
Curl/schedule.o: Curl/schedule.c Curl/schedule.h Curl/roster.h 
	$(CC) -c Curl/schedule.c -o Curl/schedule.o $(LIBS) $(GTK_LIBS)
	
UI/login.o: UI/login.c UI/login.h Helpers/jsonReader.o
	$(CC) -c UI/login.c -o UI/login.o $(GTK_LIBS)
	
UI/dmx.o: UI/dmx.c UI/dmx.h
	$(CC) -c UI/dmx.c -o UI/dmx.o $(GTK_LIBS)
	
UI/error.o: UI/error.c UI/error.h
	$(CC) -c UI/error.c -o UI/error.o $(GTK_LIBS)
	
UI/surveyUI.o: UI/surveyUI.c UI/surveyUI.h
	$(CC) -c UI/surveyUI.c -o UI/surveyUI.o $(GTK_LIBS)
	
UI/weighUI.o: UI/weighUI.c UI/weighUI.h
	$(CC) -c UI/weighUI.c -o UI/weighUI.o $(GTK_LIBS)
	
clean:
	rm -f program $(OBJS)

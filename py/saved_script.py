import cPickle as pickle

settings = {"import": "red"}

with open("settings.pickle" , 'wb') as handle:
	pickle.dump(settings, handle)

print settings

import tensorflow as tf
import numpy as np
import matplotlib as mpl
#import sklearn
#from sklearn.model_selection import train_test_split
from tensorflow import keras
import matplotlib.pyplot as plt


data = np.loadtxt("datatrue1_1M.dat")
data = data.astype('float32')
labels =  np.loadtxt("labelstimestrue1_1M.dat")
labels = labels.astype('float32')
#predictsamples=np.loadtxt("data-2signals_new-test.dat")

data_max = np.amax(data)
data_min = np.amin(data)


#data = data.astype('float32')/data_max


#train_to_test_ratio=0.8

#X_train,X_test,Y_train,Y_test=train_test_split(data,labels,train_size=train_to_test_ratio)
print (data.shape)
print (labels.shape)
nevents,nsamples = data.shape

data = data.reshape(nevents,nsamples,1)
model = keras.Sequential(
    [
        keras.layers.Input(shape=(1024, 1)),
        keras.layers.Conv1D(
            filters=32, kernel_size=16, strides=1, activation="relu", padding="same",
        ),
        keras.layers.Dropout(rate=0.1),
        keras.layers.Conv1D(
            filters=16, kernel_size=54, strides=1, activation="relu", padding="same",
        ),
        keras.layers.Conv1D(
            filters=8, kernel_size=12, strides=1, activation="relu", padding="same",
        ),
        keras.layers.Conv1DTranspose(
            filters=8, kernel_size=12, strides=1, activation="relu", padding="same",
        ),
        keras.layers.UpSampling1D(size=4),
        keras.layers.Conv1DTranspose(
            filters=16, kernel_size=56, strides=1, activation="relu", padding="same",
        ),
        keras.layers.Dropout(rate=0.1),
        keras.layers.Conv1DTranspose(
            filters=32, kernel_size=64, strides=1, activation="relu", padding="same",
        ),
        keras.layers.Conv1DTranspose(filters=1, kernel_size=16, activation="relu", padding="same"),
    ]
)
model.compile(optimizer=keras.optimizers.Nadam(learning_rate=0.00001), loss="mse")
model.summary()



pretty=np.loadtxt("datatrue2.dat")
#pretty = pretty.reshape(data.shape)


history = model.fit(
    data,
    labels,
    epochs=150,
    batch_size=1024,
    validation_split=0.1,
    #callbacks=[
        #keras.callbacks.EarlyStopping(monitor="val_loss", patience=5, mode="min")
    #],
)
#
model.save('model_truesignals')
#
#new_model = keras.models.load_model('my_model')

print(pretty.shape)
nevents_pred, nsamples_pred = pretty.shape

pretty = pretty.reshape(nevents_pred, nsamples_pred, 1)
print(pretty.shape)

bashresults=model.predict(pretty)
resfilepretty = open('testresults_truesignals_230520_16_8_4_3M.dat',"w")
np.savetxt(resfilepretty,bashresults.reshape( nevents_pred, nsamples_pred ),fmt="%i")
resfilepretty.close()



plt.plot(history.history['loss'])
plt.plot(history.history['val_loss'])
plt.title('model loss')
plt.ylabel('loss')
plt.xlabel('epoch')
plt.legend(['train', 'validation'], loc='upper left')
plt.show()

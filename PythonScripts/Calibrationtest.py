import numpy
from scipy import linalg as linsci
from numpy import array, dot
import Transformations3D as tr
from calibrationtools import Device, solveHomogeneousDualSylvester, solveWithSVD, checkOrthonormal
numpy.set_printoptions(precision=2, suppress=True)

numpy.random.seed(1)

#==============================================
#Simulation config
number_of_additional_devices = 1
number_of_samples = 10
translation_noise = 0.0


devices = []
# devices[0].deviceBaseToRefBase = tr.rotation_matrix(-2.685928751942306, array([-0.94, -0.12,  0.31]))
# devices[0].deviceToRef = tr.rotation_matrix(1.9751560849767928, array([-0.05, -0.32,  0.94]))
for i in range(number_of_additional_devices):
	devices.append(Device(deviceBaseToRefBase = dot(tr.random_rotation_matrix(), tr.translation_matrix(tr.random_vector(3))), deviceToRef = dot(tr.random_rotation_matrix(), tr.translation_matrix(tr.random_vector(3)))))
	# devices.append(Device(deviceBaseToRefBase = numpy.identity(4), deviceToRef = numpy.identity(4)))
	# devices.append(Device(deviceBaseToRefBase = dot(numpy.identity(4), tr.translation_matrix(tr.random_vector(3))), deviceToRef = dot(numpy.identity(4), tr.translation_matrix(tr.random_vector(3)))))
	# devices.append(Device(deviceBaseToRefBase = dot(tr.random_rotation_matrix(), numpy.identity(4)), deviceToRef = dot(tr.random_rotation_matrix(), numpy.identity(4))))

# list of maps from devices to device base coordinates
refSamples, deviceSamples = [], []
for i in range(number_of_samples):
	refSamples.append(dot(tr.rotation_matrix((i+1) / 6.0,[i,(i+1)%2,0]) , tr.translation_matrix([i,(i%2),0]))) #dot(tr.random_rotation_matrix() , tr.translation_matrix(tr.random_vector(3))))
	deviceSamples.append([])
	for j in range(number_of_additional_devices):
		#CORRECT
		deviceSamples[-1].append(dot(numpy.linalg.inv(devices[j].baseToRefBase), dot(refSamples[-1], devices[j].toRef))) #Compute correct secondary device positions
		deviceSamples[-1][-1] = dot(deviceSamples[-1][-1], tr.translation_matrix(translation_noise*tr.random_vector(3)))

for s in refSamples:
	if not checkOrthonormal(s):
		print "\n\n\n\n\n\n ERROR: A MATRIX IS NOT ORTHONORMAL:"
		print s," \n\n\n\n\n\n\n"
for s in deviceSamples:
	for d in s:
		if not checkOrthonormal(d):
			print "\n\n\n\n\n\n ERROR: A MATRIX IS NOT ORTHONORMAL:"
			print d," \n\n\n\n\n\n\n"


measuredBaseToRefBases =[]
allcorrect = True
number_success = 0
number_fail = 0
for i in range(number_of_additional_devices):
	measuredToRef, measuredBaseToRefBase = solveHomogeneousDualSylvester(array(refSamples),array(deviceSamples)[:,i]) # test first device for now

	correct = numpy.allclose(devices[i].baseToRefBase,measuredBaseToRefBase)
	if(not correct):
		print "devices[",i,"].deviceBaseToRefBase"
		print tr.rotation_from_matrix( devices[i].baseToRefBase )[:2]
		# print devices[i].baseToRefBase
		print " == measuredBaseToRefBase"
		# print tr.rotation_from_matrix( measuredBaseToRefBase )[:2]
		# print measuredBaseToRefBase
		print " = ", correct
		number_fail+=1
	else:
		number_success+=1

	correct = numpy.allclose(devices[i].toRef,measuredToRef)
	if(not correct):
		print "devices[",i,"].deviceToRef"
		print tr.rotation_from_matrix(devices[i].toRef)[:2]
		# print devices[i].toRef
		print "measuredToRef"
		# print tr.rotation_from_matrix(measuredToRef)[:2]
		# print measuredToRef
		print " = ", correct
		number_fail+=1
	else:
		number_success+=1


	measuredBaseToRefBases.append(measuredBaseToRefBase)

if(number_fail==0):
	print "\n\n\nSUCCESS: calibrated ", len(devices), " devices\n\n\n"
else:
	print "\n\n\n", number_fail,"/", 2*len(devices), " FAILED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n\n"

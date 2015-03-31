import numpy
from numpy import array, dot
import Transformations3D as tr
#==============================================
#Classes
class Device:
	def __init__(self, deviceBaseToRefBase = numpy.identity(4), deviceToRef = numpy.identity(4)):
		self.baseToRefBase = deviceBaseToRefBase
		self.toRef = deviceToRef
#==============================================


def kroneckerDelta(i,j):
	if (i == j):
		return 1
	else:
		return 0

def crossMatrix(v):
	"""
	Returns matrix M(v) such that for any vector x, cross(v,x) = dot(M(v),x)
	CHECKED
	"""
	return array([[0,    -v[2],  v[1]],
				  [v[2],     0, -v[0]],
				  [-v[1], v[0],    0]])

def checkOrthonormal(M):
	if M.shape[0] == 4:
		if numpy.linalg.norm(M[3,:3]) != 0:
			print "\n\n\n\n\n\n\nBottom row of matrix non-zero ", numpy.linalg.norm(M[3,:3]), "\n\n\n\n\n\n\n"
			return False
	for i in range(3):
		for j in range(3):
			if not (numpy.allclose(dot(M[:3,i],M[:3,j]), kroneckerDelta(i,j))):
				print "\n\n\n\n\n\n\nColumn ", i, " and Column ", j, " are not orthonormal: dotprod = ", dot(M[:3,i],M[:3,j]), "\n\n\n\n\n\n\n"
				return False
	return True

def solveWithSVD(A,b):
	"""
	Returns least squares solution x to Ax = b using the psuedoinverse
	CHECKED
	"""	
	return dot(numpy.linalg.pinv(A), b)

def solveHomogeneousDualSylvester(samplesA,samplesB):
	"""	
	solves AX=YB for X,Y and A in sampleA, B in sampleB

	Source:
	@ARTICLE{Zhuang1994, 
	author={Hanqi Zhuang and Roth, Zvi S. and Sudhakar, R.}, 
	journal={Robotics and Automation, IEEE Transactions on}, 
	title={Simultaneous robot/world and tool/flange calibration by solving homogeneous transformation equations of the form AX=YB}, 
	year={1994}, 
	month={Aug}, 
	volume={10}, 
	number={4}, 
	pages={549-554}
	}	
	"""
	X,Y = numpy.identity(4), numpy.identity(4)

	combinedG, combinedC = array([]),array([])
	for i in range(len(samplesA)):
		A = samplesA[i]
		B = samplesB[i]
		
		#Get Quaternions for rotations
		quat_a = tr.quaternion_from_matrix(A[:3,:3])
		a0 = quat_a[0]
		a = quat_a[1:]

		quat_b = tr.quaternion_from_matrix(B[:3,:3])
		b0 = quat_b[0]
		b = quat_b[1:]

		#Compute G in Gw = C
		if(numpy.allclose(a0, 0)):
			print "\n\n\n\n\n\n\nInvalid sampleA rotation!!!\n\n\n\n\n\n\n"
			return
		print "a0 = ", a0, " should be >> 0"

		G1 = a0 * numpy.identity(3) + crossMatrix(a) + numpy.outer(a,a) / a0
		G2 = -b0 * numpy.identity(3) + crossMatrix(b) - numpy.outer(a,b) / a0
		
		G = numpy.concatenate([G1.T, G2.T]).T

		#Compute C in Gw = C
		C = b - (b0/a0) * a

		if i == 0:
			combinedG = G
			combinedC = C
		else:
			combinedG = numpy.concatenate([combinedG,G])
			combinedC = numpy.concatenate([combinedC,C])

	w = solveWithSVD(combinedG,combinedC)

	print dot(combinedG,w) - combinedC
	
	#Compute x and y Quaternions
	x,y = numpy.zeros((4)),numpy.zeros((4))
	y[0] = 1 / numpy.sqrt(1 + w[3]*w[3] + w[4]*w[4] + w[5]*w[5])
	if numpy.allclose(y[0],0):
		print "\n\n\n\n\n\n\ny[0] == 0 so you need to rotate the ref base with respect to the base\n\n\n\n\n\n\n"
	print "y[0] = ", y[0], " should be >> 0"
	y[1:4] = y[0] * w[3:6]
	x[1:4] = y[0] * w[0:3]
	
	x0 = dot((a/a0), x[1:4]) + (b0/a0) * y[0] - dot((b/a0) , y[1:4])
	x_sign = numpy.sign(x0)	
	x[0] = x_sign * numpy.sqrt(1 - x[1]*x[1] - x[2]*x[2] - x[3]*x[3])
	# # check:
	check = tr.quaternion_multiply(tr.quaternion_inverse(tr.quaternion_multiply(quat_a,x)), tr.quaternion_multiply(y,quat_b))

	Rx, Ry = tr.quaternion_matrix(x)[:3,:3], tr.quaternion_matrix(y)[:3,:3]

	for i in range(len(samplesA)):
		RA = samplesA[i][0:3,0:3]
		pA = samplesA[i][0:3,3]
		pB = samplesB[i][0:3,3]

		F = numpy.concatenate((RA.T,-numpy.identity(3))).T

		D = dot(Ry, pB) - pA

		if i == 0:
			combinedF = F
			combinedD = D
		else:
			combinedF = numpy.concatenate([combinedF,F])
			combinedD = numpy.concatenate([combinedD,D])
		
	pxpy = solveWithSVD(combinedF,combinedD)

	X[:3,:3] = Rx 
	Y[:3,:3] = Ry

	X[:3,3] = pxpy[:3]
	Y[:3,3] = pxpy[3:6]

	return X,Y



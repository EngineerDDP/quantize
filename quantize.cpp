// ���嵼���� Python �ṹ
//

#include "pch.h"

int init_numpy() {
	import_array(); // PyError if not successful
	return 0;
}

const static int numpy_initialized = init_numpy();

// ----------------------------------------
//               �����ʼ��
// ----------------------------------------

static const char* quant_classname_string = "<Quantization class>";

static PyObject* Quant_Init(Quant* self, PyObject* pArgs, PyObject* kwds) {
	PyObject* res = Py_None;
	Py_INCREF(Py_None);

	if (self != nullptr) {

		PyObject* quant_space;
		PyArg_ParseTuple(pArgs, "O", &quant_space);
		if (!PySequence_Check(quant_space)) {
			PyErr_SetString(PyExc_TypeError, "List of float were required.");
			return res;
		}
		quant_space = PySequence_Fast(quant_space, "Iterable object is required.");

		PyObject* item;

		int len_space = PySequence_Fast_GET_SIZE(quant_space);

		/* ----------- �ڴ���� ----------- */

		double* d_quant_space = new double[len_space] {0.0};

		/* ----------- �ڴ���� ----------- */

		for (int i = 0; i < len_space; ++i) {
			item = PySequence_Fast_GET_ITEM(quant_space, i);

			if (!PyArg_Parse(item, "d", d_quant_space + i)) {
				PyErr_SetString(PyExc_TypeError, "Space must be float type.");
				return res;
			}
		}
		// ������Ķ���
		self->core_instance = new Quantize(d_quant_space, len_space);

		/* ----------- �ڴ��ͷ� ----------- */

		delete[] d_quant_space;
		Py_DECREF(quant_space);

		/* ----------- �ڴ��ͷ� ----------- */

		Py_DECREF(res);
		return 0;
	}
	else {
		PyErr_SetString(PyExc_TypeError, "Memory allocation failed.");
		return res;
	}
}

static void Quant_Destruct(Quant* self) {
	if (self->core_instance) {
		delete self->core_instance;
	}
	Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

static PyObject* Quant_Str(Quant* self) {
	return Py_BuildValue("s", quant_classname_string);
}

static PyObject* Quant_Repr(Quant* self) {
	return Quant_Str(self);
}

// ----------------------------------------
//               �����Ա����
// ----------------------------------------

static PyObject* Quant_Stochastic_Quantize(Quant* self, PyObject* pArgs) {
	PyObject* res = Py_None;
	Py_INCREF(Py_None);

	PyObject* py_float_array;
	int data_struct_len = 4;

	// ��ȡPythonObject
	PyArg_ParseTuple(pArgs, "O", &py_float_array);
	// ���ndarray����
	if (!PyArray_Check(py_float_array)) {
		PyErr_SetString(PyExc_TypeError, "ndarray were required.");
		return res;
	}
	// ������ݳ���
	if (PyArray_TYPE(py_float_array) == NPY_DOUBLE) {
		data_struct_len = 8;
	}
	else {
		PyErr_SetString(PyExc_TypeError, "dtype must be float64");
		return res;
	}

	// ���ά��
	int dims = PyArray_NDIM(py_float_array);
	if (dims != 1) {
		PyErr_SetString(PyExc_TypeError, "Can only accept 1-dimision array.");
		return res;
	}

	// ��ȡԪ����Ŀ
	npy_intp array_len = PyArray_Size(py_float_array);

	// ��ȡbuffer
	const double* input_buffer = reinterpret_cast<double*>(PyArray_DATA(py_float_array));

	/* ----------- �ڴ���� ----------- */

	char* byte_result = new char[array_len] {0};

	/* ----------- �ڴ���� ----------- */

	Py_BEGIN_ALLOW_THREADS
	// ִ�к��Ĳ���
	self->core_instance->stochastic_quantize(input_buffer, array_len, byte_result);
	Py_END_ALLOW_THREADS

	res = Py_BuildValue("y#", byte_result, array_len);

	/* ----------- �ڴ��ͷ� ----------- */

	delete[] byte_result;
	//Py_DECREF(py_float_array);

	/* ----------- �ڴ��ͷ� ----------- */

	return res;
}

static PyObject* Quant_Deterministic_Quantize(Quant* self, PyObject* pArgs) {
	PyObject* res = Py_None;
	Py_INCREF(Py_None);

	PyObject* py_float_array;
	PyArg_ParseTuple(pArgs, "O", &py_float_array);
	if (!PySequence_Check(py_float_array)) {
		PyErr_SetString(PyExc_TypeError, "List of float were required.");
		return res;
	}
	py_float_array = PySequence_Fast(py_float_array, "Iterable object is required.");

	PyObject* item;

	int len = PySequence_Fast_GET_SIZE(py_float_array);

	/* ----------- �ڴ���� ----------- */

	double* double_array = new double[len];
	char* byte_result = new char[len] {0};

	/* ----------- �ڴ���� ----------- */

	for (int i = 0; i < len; ++i) {
		item = PySequence_Fast_GET_ITEM(py_float_array, i);

		if (!PyArg_Parse(item, "d", double_array + i)) {
			PyErr_SetString(PyExc_TypeError, "Space must be float type.");
			return res;
		}
	}

	Py_BEGIN_ALLOW_THREADS
	// ִ�к��Ĳ���
	self->core_instance->deterministic_quantize(double_array, len, byte_result);
	Py_END_ALLOW_THREADS

	res = Py_BuildValue("y#", byte_result, len);

	/* ----------- �ڴ��ͷ� ----------- */

	delete[] double_array;
	delete[] byte_result;
	Py_DECREF(py_float_array);

	/* ----------- �ڴ��ͷ� ----------- */

	return res;
}

static PyObject* Quant_Decode(Quant* self, PyObject* pArgs) {


	PyObject* res = Py_None;
	Py_INCREF(Py_None);


	char* byte_buffer;
	Py_ssize_t len = 0;
	if (!PyArg_ParseTuple(pArgs, "y#", &byte_buffer, &len)) {
		return res;
	}

	/* ----------- �ڴ���� ----------- */

	double* double_array = new double[len] {0.0};

	/* ----------- �ڴ���� ----------- */

	try {
		self->core_instance->decode_quantized_array(byte_buffer, len, double_array);
	}
	catch (UnknownCodeException e) {
		PyErr_SetString(PyExc_TypeError, "Unexpected input value.");
		return res;
	}

	// д��ndarray
	res = PyArray_SimpleNewFromData(1, &len, NPY_DOUBLE, double_array);
	// �й��ڴ�
	PyArray_ENABLEFLAGS(reinterpret_cast<PyArrayObject*>(res), NPY_OWNDATA);

	/* ----------- �ڴ��ͷ� ----------- */

	// delete[] double_array;
	// Py_DECREF(res);

	/* ----------- �ڴ��ͷ� ----------- */

	return res;
}

// ----------------------------------------
//               ע��ṹ
// ----------------------------------------

static PyMemberDef Quant_DataMembers[] = {
	//{const_cast<char*>("<Cpp core encoder>"), T_NONE, offsetof(Quant, core_instance), 0, const_cast<char*>("<Cpp core encoder>")},
	{NULL, NULL, NULL, 0, NULL}
};

// ----------------------------------------
//               ע�᷽��
// ----------------------------------------

static PyMethodDef Quant_MethodMembers[] = {
	{"deterministic", reinterpret_cast<PyCFunction>(Quant_Deterministic_Quantize), METH_VARARGS, "quantize an array deterministically."},
	{"stochastic", reinterpret_cast<PyCFunction>(Quant_Stochastic_Quantize), METH_VARARGS, "quantize an array stochastically."},
	{"decode", reinterpret_cast<PyCFunction>(Quant_Decode), METH_VARARGS, "decode a quantized bytes array."},
	{NULL, NULL, NULL, NULL}
};

// ----------------------------------------
//               ע��ģ��
// ----------------------------------------

static PyMethodDef ModuleMethods[] = {
	{NULL, NULL, 0, NULL}
};

static PyModuleDef Quant_ModuleDef =
{
	PyModuleDef_HEAD_INIT,
	"quantization_module",
	"Quantization module",
	-1,
	ModuleMethods
};

static PyTypeObject Quant_ClassInfo;

// ----------------------------------------
//               �����ʼ������
// ----------------------------------------

PyMODINIT_FUNC PyInit_quantize(void) {
	PyObject* res = Py_None;

	Quant_ClassInfo.tp_new = PyType_GenericNew;
	Quant_ClassInfo.tp_name = "quantized_encode.quant";
	Quant_ClassInfo.tp_basicsize = sizeof(Quant);
	Quant_ClassInfo.tp_itemsize = 0;
	Quant_ClassInfo.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
	Quant_ClassInfo.tp_doc = "quantization encode and decode module, convert float into byts array.";
	Quant_ClassInfo.tp_str = (reprfunc)Quant_Str;
	Quant_ClassInfo.tp_init = (initproc)Quant_Init;
	Quant_ClassInfo.tp_dealloc = (destructor)Quant_Destruct;
	Quant_ClassInfo.tp_repr = (reprfunc)Quant_Repr;
	Quant_ClassInfo.tp_members = Quant_DataMembers;
	Quant_ClassInfo.tp_methods = Quant_MethodMembers;


	if (PyType_Ready(&Quant_ClassInfo) < 0) {
		PyErr_SetString(PyExc_ImportError, "class import error");
		return NULL;
	}
	res = PyModule_Create(&Quant_ModuleDef);

	if (res == 0) {
		PyErr_SetString(PyExc_ImportError, "Module create failed.");
		return NULL;
	}

	Py_INCREF(&Quant_ClassInfo);

	PyModule_AddObject(res, "quant", reinterpret_cast<PyObject*>(&Quant_ClassInfo));

	return res;
}
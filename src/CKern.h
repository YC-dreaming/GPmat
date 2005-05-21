#ifndef CKERN_H
#define CKERN_H
// This is the Kernel class.
#include <cmath>
#include "CTransform.h"
#include "CMatrix.h"
#include "CDist.h"
#include "ndlstrutil.h"
#include "ndlutil.h"
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <algorithm>
using namespace std;

const string KERNVERSION="0.1";
class CKern : public CMatinterface, public CTransformable, public CRegularisable {
 public:
  CKern()
    {}
  CKern(const CMatrix& X){}
  
  virtual ~CKern(){}
  CKern(const CKern& kern){}
  virtual CKern* clone() const=0;
  virtual void setInitParam()=0;
  virtual double diagComputeElement(const CMatrix& X, const int index) const=0;
  virtual void diagCompute(CMatrix& d, const CMatrix& X) const
    {
      assert(X.rowsMatch(d));
      assert(d.getCols()==1);
      for(int i=0; i<X.getRows(); i++)
	d.setVal(diagComputeElement(X, i), i);
    }
  virtual void diagCompute(CMatrix& d, const CMatrix& X, const vector<int> indices) const
    {
      assert(d.getRows()==indices.size());
      assert(d.getCols()==1);
      for(int i=0; i<indices.size(); i++)
	d.setVal(diagComputeElement(X, indices[i]), i);
    }
  virtual void setParam(const double, const int)=0;
  virtual void getGradX(vector<CMatrix*> g, const CMatrix& X, const CMatrix& X2) const=0;
  virtual void getDiagGradX(CMatrix& g, const CMatrix& X) const=0;
  virtual double getDiagGradXElement(const CMatrix& X, const int i, const int j) const=0;
  virtual double getWhite() const
    {
      return 0.0;
    }
  virtual double computeElement(const CMatrix& X1, const int index1,
			 const CMatrix& X2, const int index2) const=0;
  virtual void compute(CMatrix& K, const CMatrix& X1, const vector<int> indices1,
			  const CMatrix& X2, const vector<int> indices2) const
    {
      assert(K.rowsMatch(X1));
      assert(K.getCols()==X2.getRows());
      for(int i=0; i<indices1.size(); i++)
	{
	  for(int j=0; j<indices2.size(); j++)
	    {
	      assert(indices1[i]<K.getRows());
	      assert(indices2[j]<K.getCols());
	      K.setVal(computeElement(X1, indices1[i], X2, indices2[j]), indices1[i], indices2[j]);
	    }
	}
    }
  virtual void compute(CMatrix& K, const CMatrix& X) const
    {
      assert(K.rowsMatch(X));
      assert(K.isSquare());
      double k = 0.0;
      for(int i=0; i<K.getRows(); i++)
	{
	  for(int j=0; j<i; j++)
	    {
	      k = computeElement(X, i, X, j);
	      K.setVal(k, i, j);
	      K.setVal(k, j, i);
	    }
	  K.setVal(diagComputeElement(X, i), i, i);
	}	      
      K.setSymmetric(true);
    }
  virtual void compute(CMatrix& K, const CMatrix& X, const CMatrix& X2) const
    {
      assert(K.rowsMatch(X));
      assert(K.getCols()==X2.getRows());
      for(int i=0; i<K.getRows(); i++)
	{
	  for(int j=0; j<K.getCols(); j++)
	    {
	      K.setVal(computeElement(X, i, X2, j), i, j);
	    }
	}	      
    }
  virtual void getGradParams(CMatrix& g) const
    {
      // This is a dummy function
      cerr << "getGradParams should not be used in CKern" << endl;
      exit(1);
    }
  virtual void getGradParams(CMatrix& g, const CMatrix& X, const CMatrix& cvGrd) const
    {
      assert(g.getRows()==1);
      assert(g.getCols()==nParams);
      assert(X.rowsMatch(cvGrd));
      assert(cvGrd.isSquare());
      for(int i=0; i<nParams; i++)
	g.setVal(getGradParam(i, X, cvGrd), i);
      addPriorGrad(g); /// don't forget to add prior gradient at the end.
    }
  virtual double getGradParam(const int index, const CMatrix& X, const CMatrix& cvGrd) const=0;
  virtual double getParam(const int) const=0;
  virtual int addKern(CKern* kern)
    {
      cerr << "You cannot add a kernel to this kernel." << endl;
    }
  void setParams(const CMatrix& X)
    {
      for(int i=0; i<nParams; i++)
	setParam(X.getVal(i), i);
    }
  void getParams(CMatrix& X) const
    {
      for(int i=0; i<nParams; i++)
	X.setVal(getParam(i), i);

    }
  inline string getType() const
    {
      return type;
    }
  inline void setType(const string name)
    {
      type = name;
    }
  inline string getName() const
    {
      return kernName;
    }
  inline void setName(const string name)
    {
      kernName = name;
    }
  // non virtual functions.
  /*void initialiseKern(const int inDim);
  void initialiseKern(const CMatrix& X);
  void initialiseKern(const CKern& kern);*/
  inline void setInputDim(const int dim)
    {
      inputDim = dim;
      setInitParam();
    }
  inline const int getInputDim() const
    {
      return inputDim;
    }
  inline const int getNumParams() const
    {
      return nParams;
    }
  void setParamName(const string name, const int index)
    {
      assert(index>=0);
      assert(index<nParams);
      if(paramNames.size() == index)
	paramNames.push_back(name);
      else 
	{
	  if(paramNames.size()<index)
	    paramNames.resize(index+1, "no name");
	  paramNames[index] = name;
	}
    }
  virtual string getParamName(const int index) const
    {
      assert(index>=0);
      assert(index<paramNames.size());
      return paramNames[index];
    }
  virtual void writeParamsToStream(ostream& out) const;
  virtual void readParamsFromStream(istream& in);
  void getGradPrior(CMatrix& g) const;
  void getPriorLogProb(CMatrix& L) const;

  // void setWhite(double val);
  ostream& display(ostream& os) const;
#ifdef _NDLMATLAB
  CKern(mxArray* kern){}
  // returns an mxArray of the kern for use with matlab.
  virtual mxArray* toMxArray() const;
  virtual void fromMxArray(const mxArray* matlabArray);
  // Adds parameters to the mxArray.
  virtual void addParamToMxArray(mxArray* matlabArray) const;
  // Gets the parameters from the mxArray.
  virtual void extractParamFromMxArray(const mxArray* matlabArray);
#endif /* _NDLMATLAB*/
  // returns sum(sum(cvGrd.*dK/dparam)) 
  void getGradTransParams(CMatrix& g, const CMatrix& X, const CMatrix& cvGrd) const;
  bool equals(const CKern& kern, const double tol=ndlutil::MATCHTOL) const;
  
 protected:
  int nParams;
  string kernName;
  string type;
  vector<string> paramNames;
 private:
  int inputDim;
};
class CArdKern : public CKern {
#ifdef _NDLMATLAB
  virtual void addParamToMxArray(mxArray* matlabArray) const;
  // Gets the parameters from the mxArray.
  virtual void extractParamFromMxArray(const mxArray* matlabArray);
  // returns sum(sum(cvGrd.*dK/dparam)) 
#endif
 protected: 
  CMatrix scales;
};

// Compound Kernel --- This kernel combines other kernels together.
class CCmpndKern: public CKern {
 public:
  CCmpndKern();
  CCmpndKern(const int inDim);
  CCmpndKern(const CMatrix& X);
  ~CCmpndKern();
  CCmpndKern(const CCmpndKern&);
  CCmpndKern* clone() const
    {
      return new CCmpndKern(*this);
    }
  CCmpndKern(vector<CKern*> kernels);
  
  void setInitParam();
  double diagComputeElement(const CMatrix& X, const int index1) const;
  void diagCompute(CMatrix& d, const CMatrix& X) const;
  void setParam(const double val, const int paramNum);
  double getParam(const int paramNum) const;
  string getParamName(const int index) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X, const CMatrix& X2) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X) const;
  void getDiagGradX(CMatrix& g, const CMatrix& X) const;
  double getDiagGradXElement(const CMatrix& X, const int i, const int j) const;
  double getWhite() const;
  double computeElement(const CMatrix& X1, const int index1, 
		 const CMatrix& X2, const int index2) const;
  void compute(CMatrix& K, const CMatrix& X, const CMatrix& X2) const;
  void compute(CMatrix& K, const CMatrix& X) const;
  void getGradParams(CMatrix& g, const CMatrix& X, const CMatrix& cvGrd) const;
  double getGradParam(const int index, const CMatrix& X, const CMatrix& cvGrd) const;
  double priorLogProb() const;
  void addPrior(CDist* prior, const int index);


  void writeParamsToStream(ostream& out) const;
  void readParamsFromStream(istream& in);
  int addKern(CKern* kern);
#ifdef _NDLMATLAB
  // Kernel specific code for toMxArray() call.
  void addParamToMxArray(mxArray* matlabArray) const;
  // Kernel specific code for fromMxArray() call.
  void extractParamFromMxArray(const mxArray* matlabArray);
#endif
 private:
  // this is a heterogeneous container.
  vector<CKern*> components;
  

};
// White Noise Kernel.
class CWhiteKern: public CKern {
 public:
  CWhiteKern();
  CWhiteKern(const int inDim);
  CWhiteKern(const CMatrix& X);
  ~CWhiteKern();
  CWhiteKern(const CWhiteKern&);
  CWhiteKern* clone() const
    {
      return new CWhiteKern(*this);
    }
 void setInitParam();
  double diagComputeElement(const CMatrix& X, const int index) const;
  void diagCompute(CMatrix& d, const CMatrix& X) const;
  void setParam(double val, const int paramNum);
  double getParam(const int paramNum) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X, const CMatrix& X2) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X) const;
  void getDiagGradX(CMatrix& g, const CMatrix& X) const;
  double getDiagGradXElement(const CMatrix& X, const int i, const int j) const;
  double getWhite() const;
  double computeElement(const CMatrix& X1, const int index1, 
		 const CMatrix& X2, const int index2) const;
  void  compute(CMatrix& K, const CMatrix& X, const CMatrix& X2) const;
  void compute(CMatrix& K, const CMatrix& X) const;
  double getGradParam(const int index, const CMatrix& X, const CMatrix& cvGrd) const;


 private:
  double variance;
};

// Bias Kernel.
class CBiasKern: public CKern {
 public:
  CBiasKern();
  CBiasKern(const int inDim);
  CBiasKern(const CMatrix& X);
  ~CBiasKern();
  CBiasKern(const CBiasKern&);
  CBiasKern* clone() const
    {
      return new CBiasKern(*this);
    }
  void setInitParam();
  double diagComputeElement(const CMatrix& X, const int index) const;
  void diagCompute(CMatrix& d, const CMatrix& X) const;
  void setParam(double val, const int paramNum);
  double getParam(const int paramNum) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X, const CMatrix& X2) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X) const;
  void getDiagGradX(CMatrix& g, const CMatrix& X) const;
  double getDiagGradXElement(const CMatrix& X, const int i, const int j) const;
  double getWhite() const;
  double computeElement(const CMatrix& X1, const int index1,
		 const CMatrix& X2, const int index2) const;
  void compute(CMatrix& K, const CMatrix& X, const CMatrix& X2) const;
  void compute(CMatrix& K, const CMatrix& X) const;
  double getGradParam(const int index, const CMatrix& X, const CMatrix& cvGrd) const;



 private:
  double variance;

};
// RBF Kernel.
class CRbfKern: public CKern {
 public:
  CRbfKern();
  CRbfKern(const int inDim);
  CRbfKern(const CMatrix& X);
  ~CRbfKern();
  CRbfKern(const CRbfKern&);
  CRbfKern* clone() const
    {
      return new CRbfKern(*this);
    }
  void setInitParam();
  double diagComputeElement(const CMatrix& X, const int index) const;
  void diagCompute(CMatrix& d, const CMatrix& X) const;
  void setParam(double val, const int paramNum);
  double getParam(const int paramNum) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X, const CMatrix& X2) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X) const;
  void getDiagGradX(CMatrix& g, const CMatrix& X) const;
  double getDiagGradXElement(const CMatrix& X, const int i, const int j) const;
  double getWhite() const;
  double computeElement(const CMatrix& X1, const int index1, 
		  const CMatrix& X2, const int index2) const;
  void getGradParams(CMatrix& g, const CMatrix& X, const CMatrix& cvGrd) const;
  double getGradParam(const int index, const CMatrix& X, const CMatrix& cvGrd) const;

 private:
  double variance;
  double inverseWidth;

};

// Linear Kernel.
class CLinKern: public CKern {
 public:
  CLinKern();
  CLinKern(const int inDim);
  CLinKern(const CMatrix& X);
  ~CLinKern();
  CLinKern(const CLinKern&);
  CLinKern* clone() const
    {
      return new CLinKern(*this);
    }
  void setInitParam();
  double diagComputeElement(const CMatrix& X, const int index) const;
  void setParam(double val, const int paramNum);
  double getParam(const int paramNum) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X, const CMatrix& X2) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X) const;
  void getDiagGradX(CMatrix& g, const CMatrix& X) const;
  double getDiagGradXElement(const CMatrix& X, const int i, const int j) const;
  double getWhite() const;
  double computeElement(const CMatrix& X1, const int index1, 
		 const CMatrix& X2, const int index2) const;
  void compute(CMatrix& K, const CMatrix& X, const CMatrix& X2) const;
  void compute(CMatrix& K, const CMatrix& X) const;
  double getGradParam(const int index, const CMatrix& X, const CMatrix& cvGrd) const;


 private:
  double variance;
};

// MLP Kernel.
class CMlpKern: public CKern {
 public:
  CMlpKern();
  CMlpKern(const int inDim);
  CMlpKern(const CMatrix& X);
  ~CMlpKern();
  CMlpKern(const CMlpKern&);
  CMlpKern* clone() const
    {
      return new CMlpKern(*this);
    }
  void setInitParam();
  double diagComputeElement(const CMatrix& X, const int index) const;
  void setParam(double val, const int paramNum);
  double getParam(const int paramNum) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X, const CMatrix& X2) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X) const;
  void getDiagGradX(CMatrix& g, const CMatrix& X) const;
  double getDiagGradXElement(const CMatrix& X, const int i, const int j) const;
  double getWhite() const;
  double computeElement(const CMatrix& X1, const int index1, 
		  const CMatrix& X2, const int index2) const;
  void getGradParams(CMatrix& g, const CMatrix& X, const CMatrix& cvGrd) const;
  double getGradParam(const int index, const CMatrix& X, const CMatrix& cvGrd) const;

 private:
  double weightVariance;
  double biasVariance;
  double variance;
  mutable CMatrix innerProd;
};

// POLY Kernel.
class CPolyKern: public CKern {
 public:
  CPolyKern();
  CPolyKern(const int inDim);
  CPolyKern(const CMatrix& X);
  ~CPolyKern();
  CPolyKern(const CPolyKern&);
  CPolyKern* clone() const
    {
      return new CPolyKern(*this);
    }
  void setDegree(const double val)
    {
      degree = val;
    }
  double getDegree() const
    {
      return degree;
    }
  void setInitParam();
  double diagComputeElement(const CMatrix& X, const int index) const;
  void setParam(double val, const int paramNum);
  double getParam(const int paramNum) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X, const CMatrix& X2) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X) const;
  void getDiagGradX(CMatrix& g, const CMatrix& X) const;
  double getDiagGradXElement(const CMatrix& X, const int i, const int j) const;
  double getWhite() const;
  double computeElement(const CMatrix& X1, const int index1, 
		  const CMatrix& X2, const int index2) const;
  void getGradParams(CMatrix& g, const CMatrix& X, const CMatrix& cvGrd) const;
  double getGradParam(const int index, const CMatrix& X, const CMatrix& cvGrd) const;
  void writeParamsToStream(ostream& out) const;
  void readParamsFromStream(istream& in);

 private:
  double weightVariance;
  double biasVariance;
  double variance;
  double degree;
  mutable CMatrix innerProd;
};

// Linear ARD Kernel.
class CLinardKern: public CArdKern {
 public:
  CLinardKern();
  CLinardKern(const int inDim);
  CLinardKern(const CMatrix& X);
  ~CLinardKern();
  CLinardKern(const CLinardKern&);
  CLinardKern* clone() const
    {
      return new CLinardKern(*this);
    }
  void setInitParam();
  double diagComputeElement(const CMatrix& X, const int index) const;
  void setParam(double val, const int paramNum);
  double getParam(const int paramNum) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X, const CMatrix& X2) const;
  void getDiagGradX(CMatrix& g, const CMatrix& X) const;
  double getDiagGradXElement(const CMatrix& X, const int i, const int j) const;
  double getWhite() const;
  double computeElement(const CMatrix& X1, const int index1, 
		 const CMatrix& X2, const int index2) const;
  void getGradParams(CMatrix& g, const CMatrix& X, const CMatrix& cvGrd) const;
  double getGradParam(const int index, const CMatrix& X, const CMatrix& cvGrd) const;


 private:
  double variance;
};

// RBF ARD Kernel.
class CRbfardKern: public CArdKern {
 public:
  CRbfardKern();
  CRbfardKern(const int inDim);
  CRbfardKern(const CMatrix& X);
  ~CRbfardKern();
  CRbfardKern(const CRbfardKern&);
  CRbfardKern* clone() const
    {
      return new CRbfardKern(*this);
    }
  void setInitParam();
  double diagComputeElement(const CMatrix& X, const int index) const;
  void setParam(double val, const int paramNum);
  double getParam(const int paramNum) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X, const CMatrix& X2) const;
  void getDiagGradX(CMatrix& g, const CMatrix& X) const;
  double getDiagGradXElement(const CMatrix& X, const int i, const int j) const;
  double getWhite() const;
  double computeElement(const CMatrix& X1, const int index1, 
		 const CMatrix& X2, const int index2) const;
  void getGradParams(CMatrix& g, const CMatrix& X, const CMatrix& cvGrd) const;
  double getGradParam(const int index, const CMatrix& X, const CMatrix& cvGrd) const;


 private:
  double variance;
  double inverseWidth;
  mutable CMatrix gscales;
};

// MLP ARD Kernel.
class CMlpardKern: public CArdKern {
 public:
  CMlpardKern();
  CMlpardKern(const int inDim);
  CMlpardKern(const CMatrix& X);
  ~CMlpardKern();
  CMlpardKern(const CMlpardKern&);
  CMlpardKern* clone() const
    {
      return new CMlpardKern(*this);
    }
  void setInitParam();
  double diagComputeElement(const CMatrix& X, const int index) const;
  void setParam(double val, const int paramNum);
  double getParam(const int paramNum) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X, const CMatrix& X2) const;
  void getDiagGradX(CMatrix& g, const CMatrix& X) const;
  double getDiagGradXElement(const CMatrix& X, const int i, const int j) const;
  double getWhite() const;
  double computeElement(const CMatrix& X1, const int index1, 
		 const CMatrix& X2, const int index2) const;
  void getGradParams(CMatrix& g, const CMatrix& X, const CMatrix& cvGrd) const;
  double getGradParam(const int index, const CMatrix& X, const CMatrix& cvGrd) const;


 private:
  double weightVariance;
  double biasVariance;
  double variance;
  mutable CMatrix innerProd;
  mutable CMatrix gscales;
};

// Polynomial ARD Kernel.
class CPolyardKern: public CArdKern {
 public:
  CPolyardKern();
  CPolyardKern(const int inDim);
  CPolyardKern(const CMatrix& X);
  ~CPolyardKern();
  CPolyardKern(const CPolyardKern&);
  CPolyardKern* clone() const
    {
      return new CPolyardKern(*this);
    }
  void setDegree(const double val)
    {
      degree = val;
    }
  double getDegree() const
    {
      return degree;
    }
  void setInitParam();
  double diagComputeElement(const CMatrix& X, const int index) const;
  void setParam(double val, const int paramNum);
  double getParam(const int paramNum) const;
  void getGradX(vector<CMatrix*> g, const CMatrix& X, const CMatrix& X2) const;
  void getDiagGradX(CMatrix& g, const CMatrix& X) const;
  double getDiagGradXElement(const CMatrix& X, const int i, const int j) const;
  double getWhite() const;
  double computeElement(const CMatrix& X1, const int index1, 
		 const CMatrix& X2, const int index2) const;
  void getGradParams(CMatrix& g, const CMatrix& X, const CMatrix& cvGrd) const;
  double getGradParam(const int index, const CMatrix& X, const CMatrix& cvGrd) const;
  void writeParamsToStream(ostream& out) const;
  void readParamsFromStream(istream& in);


 private:
  double weightVariance;
  double biasVariance;
  double variance;
  double degree;
  mutable CMatrix innerProd;
  mutable CMatrix gscales;
};

ostream& operator<<(ostream& os, const CKern& A);
void writeKernToStream(const CKern& kern, ostream& out);
CKern* readKernFromStream(istream& in);


#endif

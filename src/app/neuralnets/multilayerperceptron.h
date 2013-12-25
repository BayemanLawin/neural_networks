#ifndef MULTILAYERPERCEPTRON_H
#define MULTILAYERPERCEPTRON_H

#include <vector>
#include <Eigen/Dense>
#include <memory>

#include "asupervisedlearning.h"
#include "utils/matrixsequence.h"
#include "utils/dimensionalexception.h"
#include "utils/math.h"

using std::shared_ptr;
using std::vector;
using std::tuple;

typedef Eigen::MatrixXd Matrix;
typedef Eigen::ArrayXd ArrayX;


class MultiLayerPerceptron : public ASupervisedLearning
{
public:
    /**
     * @brief MultiLayerPerceptron
     *  Create a new logistic perceptron with a configurable number of hidden
     *  layers.
     *
     * @param dimensions
     *  The dimension of the network. The first item is the size of the input
     *  and the last one is the size of the output. The other ones are the
     *  hidden layers.
     **/
    MultiLayerPerceptron(const vector<int> &dimensions_layers)
        :coefficients_(getShapes(dimensions_layers))
    {
        // Set dimensions_layers_.
        dimensions_layers_.reset(new vector<int>(dimensions_layers.size()));
        for(unsigned int i=0; i<dimensions_layers.size(); i++) {
            dimensions_layers_->at(i) = dimensions_layers.at(i);
        }

        // Random initialisation of the coefficients.
        // We use an empirical formula recommended by Andrew Ng.
        for(unsigned int i=0; i<dimensions_layers.size() -1; i++) {
            const Matrix& coefficient = coefficients_.matrix(i);
            double epsilon = sqrt(6.0f/ coefficient.rows() / coefficient.cols());
            coefficients_.matrix(i) = Matrix::Random(coefficient.rows(),
                                                     coefficient.cols()) * epsilon;
        }
    }


    /**
     * @brief compute
     *  Calculate the expected output of the network. One line must be one input.
     *
     * @param X
     *  The input of the network. It must have one row per observation and as
     *  many columns as the first layer of the network.
     */
    Matrix compute(const Matrix& X) const  {
        return compute_output_layers(X).last();
    }


    /**
     * @brief getGradient
     *  Compute the normalized error term and the gradient of the network, compared to the expected value.
     *
     * @param X
     *  The input of the network. It must have one row per observation and as
     *  many columns as the first layer of the network.
     * @param Y
     *  The expected output. It must have as many rows as X and as many columns
     *  as the last layer of the network.
     *
     * @return
     *  The first term of the tuple is the mean error term.
     *  The secund argument is the gradient of the error term.
     */
    virtual tuple<double, ArrayX> computeGradient(const Matrix& X,
                                                  const Matrix& Y) const {
        if(X.rows() != Y.rows()) {
            throw DimensionalException<int>(Y.rows(), X.rows(), "rows");
        }

        MatrixSequence output_layers = compute_output_layers(X);

        const Matrix& result = output_layers.last();
        double error = -1.0d/X.rows() * (
                    Y.array() * result.array().log() +
                    (1-Y.array()) * (1-result.array()).log()
                    ).sum();

        // For each observation, the corresponding row is  the derivative of the
        // error according to  the current layer's output.
        Matrix error_layer = output_layers.last().array() - Y.array();

        // We do backpropagation. At each iteration we update d and
        // gradient.matrix(i).
        MatrixSequence gradient = coefficients_;
        for(int i=coefficients_.size()-1; i>=0; i--) {
            const Matrix& input = output_layers.matrix(i);
            gradient.matrix(i) =
                    error_layer.transpose() * addOneCol(input) / X.rows();
            if(i>0) {
                Matrix tmp = error_layer * coefficients_.matrix(i);
                error_layer = tmp.rightCols(tmp.cols()-1).array()
                        *  input.array() * (1-input.array());
            }
        }

        return tuple<double, ArrayX>(error, gradient.data());
    }


    /**
     * @brief getWeights
     *  Give the current weights used by the network.
     * @return
     *  A reference to the weights. Can't live longer than the weights themselves.
     */
    MatrixSequence& getWeights() {
        return coefficients_;
    }
    const MatrixSequence& getWeights() const {
        return coefficients_;
    }

private:
    /**
     * @brief compute_output_layers
     *  Compute the ouputs of each layers of the network.
     *
     * @param X
     *  The input of the network. It must have one row per observation and as
     *  many columns as the first layer of the network.
     *
     * @return
     *  The sequence of outputs of the layers. The first output is the input,
     *  the last the network's output.
     */
    const MatrixSequence compute_output_layers(const Matrix& X) const {
        assert(dimensions_layers_->size() == coefficients_.size() +1);
        if(X.cols() != dimensions_layers_->at(0)) {
            int n = dimensions_layers_->at(0); int nn = X.cols();
            throw new DimensionalException<int>(n, nn, "cols");
        }

        vector<tuple<int, int>> dimension(dimensions_layers_->size());
        for(unsigned int i=0; i<dimension.size(); i++) {
            dimension.at(i) = tuple<int, int>(X.rows(),
                                              dimensions_layers_->at(i));
        }

        MatrixSequence output_layers = MatrixSequence(dimension);
        output_layers.matrix(0) = X;

        for(unsigned int i=0; i< output_layers.size() -1; i++) {
            const Matrix& input = output_layers .matrix(i);
            const Matrix& theta = coefficients_.matrix(i).transpose();
            // Y = f(theta * [1 Y]) , f logistic function.
            output_layers.matrix(i+1) = logistic(addOneCol(input) * theta);
        }
        return output_layers;
    }

    /**
     * @brief dimensions_layers_
     *  The size of each layers. It is shared because it can't be modified.
     */
    shared_ptr<vector<int>> dimensions_layers_;

    /**
     * @brief coefficients_
     *  The weights of the network.
     */
    MatrixSequence coefficients_;
};

#endif // MULTILAYERPERCEPTRON_H

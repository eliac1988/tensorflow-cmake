// 2018, Patrick Wieschollek <mail@patwie.com>

#include "tensorflow/core/framework/op.h"
#include "tensorflow/core/framework/shape_inference.h"

namespace tensorflow {

using ::tensorflow::shape_inference::ShapeHandle;
using ::tensorflow::shape_inference::InferenceContext;

namespace shape_inference {
Status UnchangedShape(InferenceContext* c) {
  c->set_output(0, c->input(0));
  return Status::OK();
}
} /* shape_inference */

REGISTER_OP("MatrixAdd")
    .Attr("bias: float")
    .Attr("T: realnumbertype = DT_FLOAT")
    .Input("a: T")
    .Input("b: T")
    .Output("output: T")
    .SetShapeFn([](::tensorflow::shape_inference::InferenceContext* c) {
      // we require the input to have 4 axes
      ShapeHandle shape_hnd;
      TF_RETURN_IF_ERROR(c->WithRank(c->input(0), 4, &shape_hnd));
      TF_RETURN_IF_ERROR(c->WithRank(c->input(1), 4, &shape_hnd));

      ShapeHandle a_shape = c->input(0);
      ShapeHandle b_shape = c->input(1);

      // assert shapes of a and b are matching
      TF_RETURN_IF_ERROR(c->Merge(a_shape, b_shape, &a_shape));

      // specify output-shape
      // this could be "c->set_output(0, a_shape);"
      // but we do it explicitly
      auto B = c->Dim(c->input(0), 0);
      auto M = c->Dim(c->input(0), 1);
      auto N = c->Dim(c->input(0), 2);
      auto C = c->Dim(c->input(0), 3);
      c->set_output(0, c->MakeShape({B, M, N, C}));

      // we can also use the Attr here
      float bias;
      (void)c->GetAttr("bias", &bias);

      return Status::OK();
    })
    .Doc(R"doc(
Add two matrices and a constant

This computes `A`+`B`+`bias` for two matrices.

a: A batch of matrices [B, M, N, C].
b: A batch of matrices [B, M, N, C].
bias: An additional constant term.
output: A batch of matrices [B, M, N, C] containing the sum plus bias.
)doc");

REGISTER_OP("MatrixAddGrad")
    .Attr("bias: float")
    .Input("a: T")
    .Input("b: T")
    .Input("gradients: T")
    .Output("grad_a: T")
    .Output("grad_matrix_b: T")
    .Attr("T: realnumbertype")
    .SetShapeFn([](InferenceContext* c) {
      c->set_output(0, c->input(0));  // grad_a has same shape as a
      c->set_output(1, c->input(1));  // grad_b has same shape as b
      return ::tensorflow::Status::OK();
    })
    .Doc(R"doc(
Returns gradients of "a + b + bias".
)doc");

} /* tensorflow */

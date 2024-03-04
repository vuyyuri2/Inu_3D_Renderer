#include "model_internal.h"

#include <vector>

static std::vector<model_t> models;

int register_model(model_t& model) {
  model.id = models.size();
  models.push_back(model);
  return model.id;
}

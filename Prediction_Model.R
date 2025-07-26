# Step 1: Load required libraries and dataset
library(randomForest)
library(caret)

data <- read.csv("feeds.csv")

# Step 2: Convert fields to numeric and handle missing values using mean
fields <- c("temperature", "humidity", "co2")  # Replace with actual column names
data[fields] <- lapply(data[fields], function(x) as.numeric(as.character(x)))
data[fields] <- lapply(data[fields], function(x) {
  x[is.na(x)] <- mean(x, na.rm = TRUE)
  return(x)
})

# Step 3: Define comfort level calculation function
calculate_comfort_level <- function(temp, humidity, co2) {
  temp_range <- c(22, 26)
  humidity_range <- c(40, 60)
  co2_range <- c(400, 700)
  
  temp_dev <- if (temp < temp_range[1]) temp_range[1] - temp else if (temp > temp_range[2]) temp - temp_range[2] else 0
  humidity_dev <- if (humidity < humidity_range[1]) humidity_range[1] - humidity else if (humidity > humidity_range[2]) humidity - humidity_range[2] else 0
  co2_dev <- if (co2 < co2_range[1]) co2_range[1] - co2 else if (co2 > co2_range[2]) co2 - co2_range[2] else 0
  
  score <- temp_dev + humidity_dev + co2_dev
  
  if (score == 0) {
    return(1)
  } else if (score <= 5) {
    return(2)
  } else if (score <= 10) {
    return(3)
  } else {
    return(4)
  }
}

# Step 4: Apply comfort level calculation to each row
data$comfort_level <- mapply(
  calculate_comfort_level,
  data$temperature,
  data$humidity,
  data$co2
)

# Step 5: Split data into training and testing sets (70/30)
set.seed(123)
train_index <- createDataPartition(data$comfort_level, p = 0.7, list = FALSE)
train_data <- data[train_index, ]
test_data <- data[-train_index, ]

# Step 6: Train Random Forest model
rf_model <- randomForest(
  comfort_level ~ temperature + humidity + co2,
  data = train_data,
  ntree = 100
)

# Step 7: Evaluate model accuracy on test data
predictions <- predict(rf_model, test_data)
conf_mat <- confusionMatrix(as.factor(predictions), as.factor(test_data$comfort_level))
print(conf_mat)

# Step 8: Predict comfort level for new data
new_data <- data.frame(temperature = 24, humidity = 50, co2 = 600)  
predicted_comfort <- predict(rf_model, new_data)
print(predicted_comfort)
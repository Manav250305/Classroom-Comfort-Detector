# Step 1
data <- read.csv("your_data.csv")

# Step 2
data$field1[is.na(data$field1)] <- mean(data$field1, na.rm = TRUE)
data$field2[is.na(data$field2)] <- mean(data$field2, na.rm = TRUE)
data$field3[is.na(data$field3)] <- mean(data$field3, na.rm = TRUE)

# Step 3
calculate_comfort_level <- function(temp, humidity, co2) {
  temp_comfort_range <- c(22, 26)
  humidity_comfort_range <- c(40, 60)
  co2_comfort_range <- c(400, 700)
  
  temp_deviation <- if (temp < temp_comfort_range[1]) {
    temp_comfort_range[1] - temp
  } else if (temp > temp_comfort_range[2]) {
    temp - temp_comfort_range[2]
  } else {
    0
  }
  
  humidity_deviation <- if (humidity < humidity_comfort_range[1]) {
    humidity_comfort_range[1] - humidity
  } else if (humidity > humidity_comfort_range[2]) {
    humidity - humidity_comfort_range[2]
  } else {
    0
  }
  
  co2_deviation <- if (co2 < co2_comfort_range[1]) {
    co2_comfort_range[1] - co2
  } else if (co2 > co2_comfort_range[2]) {
    co2 - co2_comfort_range[2]
  } else {
    0
  }
  
  comfort_score <- temp_deviation + humidity_deviation + co2_deviation
  
  if (comfort_score == 0) {
    return(1)
  } else if (comfort_score <= 5) {
    return(2)
  } else if (comfort_score <= 10) {
    return(3)
  } else {
    return(4)
  }
}

# Step 4
data$comfort_level <- mapply(
  calculate_comfort_level,
  data$field1,
  data$field2,
  data$field3
)

# Step 5
model <- lm(comfort_level ~ field1 + field2 + field3, data = data)

# Step 6
summary(model)
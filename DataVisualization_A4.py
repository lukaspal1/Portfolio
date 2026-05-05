#using flask we also get render template for the html, and request and json for updating the data without refreshing the page
from flask import Flask, render_template, request, jsonify
import pandas as pd

app = Flask(__name__)

#load the data
df = pd.read_csv('COVID_Country_Sample.csv', parse_dates=['date'])

#print the first 10 data markers and info to inspect data
print(df.head(10))
print(df.info())

#for new cases and new vaccinations fill missing values with 0
df['new_cases'] = pd.to_numeric(df['new_cases'], errors='coerce').fillna(0)
df['new_vaccinations'] = pd.to_numeric(df['new_vaccinations'], errors='coerce').fillna(0)



#for outliers remove any values that are 3 standard deviations or more from the data
for col in ['new_cases', 'new_vaccinations']:
    mean, std = df[col].mean(), df[col].std()
    df[col] = df[col].clip(upper=mean + 3 * std)

#put the cleaned data into a csv
df.to_csv('COVID_Country_Cleaned.csv', index=False)

#when the user visits the homepage, run index
#sorts the countries into a list for the dropdown
#then get the earliest and latest date for the top of the page
#the call the template calling the countries and data range
@app.route('/')
def index():
    countries = sorted(df['country'].unique().tolist())
    date_range = f"{df['date'].min().date()} to {df['date'].max().date()}"
    return render_template('index.html', countries=countries, date_range=date_range)

#the json endpoint
#country and metric handle the requests made whenever the dropdown changes countries or metrics
#filtered reads the values from the url
@app.route('/data')
def data():
    country = request.args.get('country', df['country'].iloc[0])
    metric = request.args.get('metric', 'new_cases')
    
    filtered = df[df['country'] == country][['date', metric]].copy()
    #change the date type to str so there are no errors with json
    filtered['date'] = filtered['date'].astype(str)
    
    #create the rolling average
    filtered['rolling'] = filtered[metric].rolling(7, min_periods=1).mean().round(2)
    
    #filter to a dictionary of lists, and send it as json to browser
    return jsonify(filtered.to_dict(orient='list'))

#only start the server if you are running the file directly
#also auto restarts when code updates
if __name__ == '__main__':
    app.run(debug=True)

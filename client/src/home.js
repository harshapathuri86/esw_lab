import { Component } from "react";
import axios from "axios";
import { Nav, Navbar } from "react-bootstrap";

export default class Home extends Component{
    constructor(props) {
        super(props);
        this.state={
      feeds:[],
        }
    }
    componentDidMount(){
      axios.get('https://api.thingspeak.com/channels/1494903/feeds.json')
        .then(response=>{
          this.setState({feeds:response.data.feeds.reverse()});
          console.log("state",this.state);
        })
        .catch(function(error){
          console.log(error);
        })
    }
    render() {
	    
        return (
            <div>
              <Navbar collapseOnSelect expand="lg" bg="dark" variant="dark">
                <Navbar.Toggle aria-controls="responsive-navbar-nav"/>
                <Navbar.Collapse id="responsive-navbar-nav">
                  <Nav className="mr-auto">
                  <Nav.Link href="/" className="active">Home</Nav.Link>
                  <Nav.Link href="/livedata" >Live data</Nav.Link>
                  <Nav.Link href="/plotdata" >Plot data</Nav.Link>
                  </Nav>
                </Navbar.Collapse>
              </Navbar>
              <br/><h2>latest 100 samples from thingspeak</h2><br/>
                <table className="table table-striped table-borderless table-hover" >
		<thead className="thead-dark">
              <tr>
		<th scope="col">#</th> 
                <th scope="col">time</th>
                <th scope="col">temperature (°c)</th>
                <th scope="col">humidity (%)</th>
              </tr>
		</thead>
		<tbody>
          {this.state.feeds.map((element)=>{
		  var date = new  Date(element.created_at);
            return <tr key={element.entry_id}>
		<th scope="row">{element.entry_id}</th>
              <td>{date.toLocaleString()}</td>
              <td>{element.field1}</td>
              <td>{element.field2}</td>
              </tr>
          })}
		</tbody>
            </table>
                </div>
        );
    }
}
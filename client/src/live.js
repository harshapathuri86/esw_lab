import { Component } from "react";
import axios from "axios";
import { Navbar, Nav } from "react-bootstrap";

export default class Livedata extends Component{
    constructor(props) {
        super(props);
        this.state={
		temperature:"",
		humidity:"",
        }
    }
    componentDidMount(){
      axios.get('http://192.168.55.101/live')
        .then(response=>{
          this.setState({...response.data});
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
                <Navbar.Brand href="">
                  <logo alt="" width="30" height="30" className="d-inline-block align-top"/>
                </Navbar.Brand>
                <Navbar.Toggle aria-controls="responsive-navbar-nav"/>
                <Navbar.Collapse id="responsive-navbar-nav">
                  <Nav className="mr-auto">
                  <Nav.Link href="/"> Home</Nav.Link>
                  <Nav.Link href="/livedata" className="active" >Live data</Nav.Link>
                  <Nav.Link href="/plotdata" >Plot data</Nav.Link>
                  </Nav>
                </Navbar.Collapse>
              </Navbar>
              <br/><h2> LiveData</h2><br/>
		<dl class="row">
		<dt class="col-sm-3">Temperature</dt>
		  <dd class="col-sm-9">{this.state.temperature} °C</dd>

		<dt class="col-sm-3">Humidity</dt>
		  <dd class="col-sm-9">{this.state.humidity} %</dd>
		</dl>
                </div>
        );
    }
}

using UnityEngine;
using System.Collections;

public class ObstacleMove : MonoBehaviour 
{	
	void Update () 
	{
        Vector3 input = new Vector3(Input.GetAxis("Horizontal"), 0, Input.GetAxis("Vertical"));
        transform.position = new Vector3(transform.position.x + (input.x * 0.1f), 0, transform.position.z + (input.z*0.1f));

       // transform.Rotate(0, Input.GetAxis("Mouse Y"), 0);
    }
}
using UnityEngine;
using System.Collections;

public class ObstacleMoveTo : MonoBehaviour
{
	private NavMeshAgent navAgent;
	private bool select;
	private bool set = false;
	private Vector3 MoveD;

	void Start ()
    {
		navAgent = GetComponent<NavMeshAgent>();
	}

	void Update ()
    {
        if (select)
        {
            Vector3 input = new Vector3(Input.GetAxis("Horizontal"), 0, Input.GetAxis("Vertical"));
            transform.position = new Vector3(transform.position.x + input.x, transform.position.y, transform.position.z + input.z);

            transform.Rotate(0, Input.GetAxis("Mouse Y"), 0);
        }
	}
	
	void Select()
	{
		Debug.Log("Cube Logged");
		select = true;
	}
	
	void Deselect(int x)
	{
		Debug.Log("Cube Delogged");
        this.transform.tag = "Cube";
		select = false;
	}

	void Destination (Vector3 dest)
	{
		Debug.Log("Destination Set");
		MoveD = dest;
		set = true;
	}
}
